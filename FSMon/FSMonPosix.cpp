#include "FSMon.h"

NAMESPACE_UPP

// code for POSIX platform
#ifdef PLATFORM_POSIX
	
#include <sys/inotify.h>
#include <sys/ioctl.h>

// sets error code message from errno
String FSMon::GetErrorStr(int err)
{
	return strerror(err);
}


// constructor
FSMon::FSMon(bool nOnClose)
{
	errCode = 0;
	errMsg = "";
	
	notifyOnClose = nOnClose;
	
	iNotifyHandle = inotify_init1(IN_NONBLOCK);

	if(iNotifyHandle == -1)
	{
		SetError(errno);
		return;
	}
	
	// no pending moves
	pendingMovePath = "";
	pendingMoveCookie = -1;
	
	// start monitor thread
	shutDown = false;
	fsmThread.Start(THISBACK(monitorCb));
}

// destructor
FSMon::~FSMon()
{
	// stops monitoring thread
	shutDown = true;
	while(shutDown)
		;
	
	if(iNotifyHandle >= 0)
	{
		// close notify handle
		// that should un-monitor all paths...
		close(iNotifyHandle);
	}
}

/*
struct inotify_event {
    int      wd;       // Watch descriptor
    uint32_t mask;     // Mask of events
    uint32_t cookie;   // Unique cookie associating related events (for rename(2))
    uint32_t len;      // Size of name field
    char     name[];   // Optional null-terminated name
};
*/

// scans a newly created folder to look for files
// being created BEFORE notify handler was in place
void FSMon::ScanCreatedFolder(String path)
{
	FindFile ff(AppendFileName(path, "*"));
	while(ff)
	{
		if(ff.IsFolder())
			ScanCreatedFolder(ff.GetPath());
		else
		{
			INTERLOCKED_(fsmMutex)
			{
				changed.Add();
				Info &info = changed.Top();
				info.flags = FSM_Created;
				info.path = ff.GetPath();
				info.newPath.Clear();
				callHandlerCb();
			}
		}
		ff.Next();
	}
}

// callback to call event handler in maint thread
// (via PostCallback) when using GUI
void FSMon::callHandlerCb(void)
{
#ifdef flagGUI
	PostCallback(EventHandler);
#else
	EventHandler();
#endif
}

// event handling selector
void FSMon::EventsSelector(uint32 mask, uint32 cookie, String const &path)
{
	int iMask = 0;
			
	// flag stating event related to folder, not file
	bool isFolder = mask & IN_ISDIR;
			
	// mask field is a bitmask with OR-ed values, so we can't use a switch
	// we just check each value in a given order
	if(mask & IN_CLOSE_WRITE)
	{
DLOG("IN_CLOSE_WRITE '" << path << "'");
		INTERLOCKED_(fsmMutex)
		{
			// remove file from opened list
			int iOpened = openedFiles.Find(path);
			if(iOpened >= 0)
				openedFiles.Remove(iOpened);

			// we propagate IN_CLOSE_WRITE event only if
			// we want to be notified just after file closing
			// otherwise, the change is always propagated by IN_MODIFY
			if(notifyOnClose)
			{
				changed.Add();
				Info &info = changed.Top();
				info.flags = FSM_Modified;
				info.path = path;
				info.newPath.Clear();
				callHandlerCb();
			}
		}
		return;
	}
	if(mask & IN_CLOSE_NOWRITE)
	{
DLOG(++iMask << " IN_CLOSE_NOWRITE '" << path << "'");
		// just update the opened files list
		int iOpened = openedFiles.Find(path);
		if(iOpened >= 0)
		{
			INTERLOCKED_(fsmMutex)
			{
				openedFiles.Remove(iOpened);
			}
		}
		return;
	}
	if(mask & IN_OPEN)
	{
DLOG(++iMask << " IN_OPEN '" << path << "'");
		// just update the opened files list
		if(openedFiles.Find(path) < 0)
		{
			INTERLOCKED_(fsmMutex)
			{
				openedFiles.Add(path);
			}
		}
		return;
	}
	if(mask & IN_CREATE)
	{
DLOG(++iMask << " IN_CREATE '" << path << "'");
		// signal file/path creation
		INTERLOCKED_(fsmMutex)
		{
			changed.Add();
			Info &info = changed.Top();
			info.flags = (isFolder ? FSM_FolderCreated : FSM_Created);
			info.path = path;
			info.newPath.Clear();
			callHandlerCb();
		}
		// if a folder was created, we shall first setup a monitor
		// in it, then ensure that in the meanwhile no subitems have been created
		if(isFolder)
		{
			INTERLOCKED_(fsmMutex2)
			{
				AddWatch(path);
			}
			ScanCreatedFolder(path);
		}
		return;
	}
	if(mask & IN_DELETE)
	{
DLOG(++iMask << " IN_DELETE '" << path << "'");
		// signal file removal
		INTERLOCKED_(fsmMutex)
		{
			changed.Add();
			Info &info = changed.Top();
			info.flags = FSM_Deleted;
			info.path = path;
			info.newPath.Clear();
			callHandlerCb();
		}
		// for folders, we shall de-monitor all contained ones
		if(isFolder)
		{
			INTERLOCKED_(fsmMutex)
			{
				for(int iFolder = monitoredPaths.GetCount() - 1; iFolder >= 0; iFolder--)
				{
					if(path.EndsWith(monitoredPaths[iFolder]))
					{
						monitoredPaths.Pop();
						int desc = monitoredDescriptors.Pop();
						inotify_rm_watch(iNotifyHandle, desc);
					}
				}
			}
		}
		return;
	}
	if(mask & IN_MODIFY)
	{
DLOG(++iMask << " IN_MODIFY '" << path << "'");
		// if we want just notifies on close, do nothing
		if(!notifyOnClose)
		{
			INTERLOCKED_(fsmMutex)
			{
				changed.Add();
				Info &info = changed.Top();
				info.flags = FSM_Modified;
				info.path = path;
				info.newPath.Clear();
				callHandlerCb();
			}
		}
		return;
	}
	if(mask & IN_MOVED_FROM)
	{
DLOG(++iMask << " IN_MOVED_FROM '" << path << "'");
		// for IN_MOVED_FROM, we must wait if an IN_MOVED_TO event follows
		// in that case, it's a move between watched folders, otherwise
		// we handle as an IN_DELETE command
		pendingMoveCookie = cookie;
		pendingMovePath = path;
		return;
	}
	if(mask & IN_MOVED_TO)
	{
DLOG(++iMask << " IN_MOVED_TO '" << path << "'");
		// we shall differentiate if it's a move from outside
		// or a move between watched folders
		// in former case, we shall take it as an IN_CREATE event; 
		// in latter case we take it as as an IN_MOVE event
		if(pendingMoveCookie == cookie)
		{
			// it's a true move from inside monitored paths
DLOG("MOVE BETWEEN MONITORE PATHS");			
			INTERLOCKED_(fsmMutex)
			{
				changed.Add();
				Info &info = changed.Top();
				info.flags = FSM_Moved;
				info.path = pendingMovePath;
				info.newPath = path;
				callHandlerCb();
				
				// if we moved a folder, we shall update stored info
				// with new ones -- we assume that monitors will stay in place...
				if(isFolder)
				{
					INTERLOCKED_(fsmMutex2)
					{
						Index<String> paths;
						for(int i = 0; i < monitoredPaths.GetCount(); i++)
						{
							String oldPth = monitoredPaths[i];
							if(oldPth.StartsWith(pendingMovePath))
								oldPth = path + oldPth.Mid(pendingMovePath.GetCount());
							paths.Add(oldPth);
						}
						monitoredPaths = paths;
					}
				}
			}
		}
		else
		{
			// it's a create event, files comes from outside
DLOG("Move is a CREATE, re-entering EventsSelector");
			EventsSelector(IN_CREATE | (isFolder ? IN_ISDIR : 0), cookie, path);
		}
		
		// reset pending ops data
		pendingMoveCookie = -1;
		pendingMovePath = "";
		return;
	}
	if(mask & IN_ATTRIB)
	{
DLOG(++iMask << " IN_ATTRIB '" << path << "'");
		// add the attribute-modify event
		INTERLOCKED_(fsmMutex)
		{
			changed.Add();
			Info &info = changed.Top();
			info.flags = FSM_AttribChange;
			info.path = path;
			info.newPath.Clear();
			callHandlerCb();
		}
	}
		return;
}

// monitoring callback (runs in a separate thread)
void FSMon::monitorCb(void)
{
	const size_t BASE_BUFSIZE = offsetof(struct inotify_event, name);

	byte *bigBuf;
	while(!shutDown)
	{
		// check if data is available from inotify
		size_t nBytes;
		if(ioctl(iNotifyHandle, FIONREAD, &nBytes) < 0)
		{
			Sleep(100);
			continue;
		}
		if(nBytes == 0 || (int)nBytes == -1)
		{
			Sleep(100);
			continue;
		}
		// buffer can be filled by many events at once; the variable-lenght
		// name overcomplicates stuffs, as usual
		// we can't also read partial data, because read returns error if buffer is too small
		// so, we have to read full available data and split records manually
		bigBuf = (byte *)malloc(nBytes);
		size_t res = read(iNotifyHandle, bigBuf, nBytes);
		if(res != nBytes)
		{
			free(bigBuf);
			Sleep(100);
			continue;
		}
		struct inotify_event *buf = (struct inotify_event *)bigBuf;
		while((byte *)buf - bigBuf < (int)nBytes)
		{
			int wd			= buf->wd;
			uint32_t mask	= buf->mask;
			uint32_t cookie	= buf->cookie;
			String name = (buf->len ? buf->name : "");
			buf = (struct inotify_event *)((byte *)buf + BASE_BUFSIZE + buf->len);
	
			// skip ignored events
			if(mask & IN_IGNORED)
			{
	DLOG("Ignored event");
				continue;
			}
			
			// get path from descriptor
			int idx = monitoredDescriptors.Find(wd);
			if(idx < 0)
			{
	DLOG("Couldn't find descriptor in stored table");
				continue;
			}
			String path = AppendFileName(monitoredPaths[idx], name);
	
			// special handling for IN_MOVE events... 
			if((int)pendingMoveCookie != -1 && !(mask & IN_MOVED_TO))
			{
				// previous MOVE_FROM was outside monitored paths
				// so we shall emit a DELETED event
				EventsSelector(IN_DELETE | (mask & IN_ISDIR ? IN_ISDIR : 0), pendingMoveCookie, pendingMovePath);
				pendingMoveCookie = -1;
				pendingMovePath = "";
			}
			
			// handle the event
			EventsSelector(mask, cookie, path);
			
		}
		free(bigBuf);
	}
	shutDown = false;
}
		
/* inofify MASKS
IN_ACCESS			File was accessed (read) (*).
IN_ATTRIB			Metadata changed, e.g., permissions, timestamps, extended attributes, link count (since Linux 2.6.25), UID, GID, etc. (*).
IN_CLOSE_WRITE		File opened for writing was closed (*).
IN_CLOSE_NOWRITE	File not opened for writing was closed (*).
IN_CREATE			File/directory created in watched directory (*).
IN_DELETE			File/directory deleted from watched directory (*).
IN_DELETE_SELF		Watched file/directory was itself deleted.
IN_MODIFY			File was modified (*).
IN_MOVE_SELF		Watched file/directory was itself moved.
IN_MOVED_FROM		File moved out of watched directory (*).
IN_MOVED_TO			File moved into watched directory (*).
IN_OPEN				File was opened (*). 
*/
// recursively add or remove monitors for paths
// try to monitor all he can, even if there are some errors
// returns false if some error is found
bool FSMon::AddWatch(String const &path)
{
	bool res = true;
	
	INTERLOCKED_(fsmMutex2)
	{
		// add a monitor to current path if not already there
		if(monitoredPaths.Find(path) >= 0)
			return true;
		
		int desc = inotify_add_watch(
			iNotifyHandle,
			path,
			IN_ATTRIB |
			IN_OPEN |				// this one just to see if file is busy
			IN_CLOSE_WRITE |
			IN_CLOSE_NOWRITE |
			IN_CREATE |
			IN_DELETE |
			// IN_DELETE_SELF |		// we handle this one in IN_DELETE anyways
			IN_MODIFY |
			// IN_MOVE_SELF |		// we handle this one in IN_MOVE anyways
			IN_MOVED_FROM |			// without IN_MOVE_TO, is a DELETE from watched folders
			IN_MOVED_TO
		);
		
		// error ?
		if(desc < 0)
		{
			errMap.Add(path, errno);
			SetError(errno);
DLOG("Error adding monitor for '" << path << "', " << errMsg);
			res = false;
		}
		else
		{
DLOG("Monitor for '" << path << "' added, descriptor is " << desc);
			monitoredPaths.Add(path);
			monitoredDescriptors.Add(desc);
		}
	
		// look for all subfolders
		FindFile ff(AppendFileName(path, "*"));
		while(ff)
		{
			if(ff.IsFolder())
				res &= AddWatch(ff.GetPath());
			ff.Next();
		}
	}
	
	return res;
}

bool FSMon::RemoveWatch(String const &path)
{
	bool res = true;
	
	int pDescIdx = monitoredPaths.Find(path);
	if(pDescIdx >= 0)
	{
		if(inotify_rm_watch(iNotifyHandle, monitoredDescriptors[pDescIdx]) < 0)
		{
			errMap.Add(path, errno);
			SetError(errno);
			res = false;
		}
		// look for all subfolders
		FindFile ff(AppendFileName(path, "*"));
		while(ff)
		{
			if(ff.IsFolder())
				res &= RemoveWatch(ff.GetPath());
			ff.Next();
		}
		
		// remove from list of monitored paths
		monitoredPaths.Remove(pDescIdx);
		monitoredDescriptors.Remove(pDescIdx);
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////

// add a monitored path
bool FSMon::Add(String const &path)
{
	if(IsError())
		return false;
	
	// check wether folder exists
	if(!DirectoryExists(path))
		return false;
	
	// clears map of errors
	errMap.Clear();
	
	// we don't want monitor again an already monitored path
	// so we do a quick check about it
	if(monitoredPaths.Find(path) >= 0)
		return true;
	
	// if we're monitoring an external path, we should unmonitor
	// it and remonitor the external one; as doing so we could loose
	// events, we just monitor the external path, that do not harm
	return AddWatch(path);
}

// remove a monitored path
bool FSMon::Remove(String const &path)
{
	if(IsError())
		return false;
	
	// check wether folder exists
	// if not, return success anyways
	if(!DirectoryExists(path))
		return true;
	
	// clears map of errors
	errMap.Clear();
	
	// do the recursive un-monitoring
	return RemoveWatch(path);
	
}

// query for changed files/folders
Vector<FSMon::Info> FSMon::GetChanged(void)
{
	Vector<Info> info;
	INTERLOCKED_(fsmMutex)
	{
		info = changed;
		changed.Clear();
	}
	return info;
}

// gets actually opened files
Index<String> FSMon::GetOpenedFiles(void)
{
	Index<String> of;
	INTERLOCKED_(fsmMutex)
	{
		of <<= openedFiles;
	}
	return of;
}
		
VectorMap<String, int> FSMon::GetErrorMap(void)
{
	VectorMap<String, int> res = errMap;
	errMap.Clear();
	return res;
}

#endif

END_UPP_NAMESPACE