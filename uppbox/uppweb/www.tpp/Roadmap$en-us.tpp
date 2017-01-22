topic "Roadmap";
[2 $$0,0#00000000000000000000000000000000:Default]
[l288;i1120;a17;O9;~~~.1408;2 $$1,0#10431211400427159095818037425705:param]
[a83;*R6 $$2,5#31310162474203024125188417583966:caption]
[b83;*4 $$3,5#07864147445237544204411237157677:title]
[i288;O9;C2 $$4,6#40027414424643823182269349404212:item]
[b42;a42;ph2 $$5,5#45413000475342174754091244180557:text]
[l288;b17;a17;2 $$6,6#27521748481378242620020725143825:desc]
[l321;t246;C@5;1 $$7,7#20902679421464641399138805415013:code]
[b2503;2 $$8,0#65142375456100023862071332075487:separator]
[*_@(0.0.255)2 $$9,0#83433469410354161042741608181528:base]
[t4167;C2 $$10,0#37138531426314131251341829483380:class]
[l288;a17;*1 $$11,11#70004532496200323422659154056402:requirement]
[i417;b42;a42;O9;~~~.416;2 $$12,12#10566046415157235020018451313112:tparam]
[b167;C2 $$13,13#92430459443460461911108080531343:item1]
[i288;a42;O9;C2 $$14,14#77422149456609303542238260500223:item2]
[*@2$(0.128.128)2 $$15,15#34511555403152284025741354420178:NewsDate]
[l321;*C$7;2 $$16,16#03451589433145915344929335295360:result]
[l321;b83;a83;*C$7;2 $$17,17#07531550463529505371228428965313:result`-line]
[l160;t4167;*C+117 $$18,5#88603949442205825958800053222425:package`-title]
[2 $$19,0#53580023442335529039900623488521:gap]
[t4167;C2 $$20,20#70211524482531209251820423858195:class`-nested]
[b50;2 $$21,21#03324558446220344731010354752573:Par]
[b125;a21;*2 $$22,22#72A57D7B347482931820FBB21B426750:subtitle]
[l42;b42;a42;phO0;2 $$23,23#42C8ED58A1FE5FAB0FCEA41746BFF560:point]
[b83;*_2 $$24,24#F44F66EA0BD27C61EACACB2D8ED88A05:release]
[{_}%EN-US 
[s2; Roadmap&]
[s24; Upcoming release&]
[s3; 2017.1 (rev 10564`+) (Jan 2017)&]
[s5; This is the first release of U`+`+ that [*/ requires] C`+`+11 
compatible compiler. The main focus was [* Core], adding many enhancements 
allowed by C`+`+11 and improved parallel programming support. 
We, also didn`'t forget about Android and we ported Core library 
on that platform.&]
[s22; Core&]
[s5;l160;i150;O0; Original U`+`+ Callbacks are deprecated and replaced 
with Function with better lambda support.&]
[s5;l160;i150;O0; Begin/End methods are now renamed / deprecated 
in favor of (standard) begin/end.&]
[s5;l160;i150;O0; U`+`+ algorithms now primarily work on [/ ranges], 
besides container SubRange, ConstRange and ViewRange are now 
provided.&]
[s5;l160;i150;O0; Initial round of optimizations for ARM architecture.&]
[s5;l160;i150;O0; U`+`+ allocator optimized once again, locking is 
reduced in inter`-thread deallocations, allocator now returns 
blocks always 16 bytes aligned (simplifies SSE2 code).&]
[s5;l160;i150;O0; General cleanup of U`+`+ algorithms, redundant 
algorithms removed, new Count and CountIf algorithms&]
[s5;l160;i150;O0; Improvements in Core/RPC&]
[s5;l160;i150;O0; CoWork`::FinLock now provides `'free of charge`' 
mutex at the end of worker jobs.&]
[s5;l160;i150;O0; CoWorkerResources class now provides `'per worker 
thread`' context.&]
[s5;l160;i150;O0; CoPartition algorithm useful for partitioning array 
for parallel programming.&]
[s5;l160;i150;O0; Where it makes sense, U`+`+ algorithms now have 
parallel variant.&]
[s5;l160;i150;O0; String further optimized, new TrimLast, TrimStart, 
TrimEnd, FindAfter, ReverseFindAfter&]
[s5;l160;i150;O0; FastCompress (currently using LZ4) for internal 
program compression.&]
[s5;l160;i150;O0; Core can be compiled on Android and major of features 
are currently ported.&]
[s5;l160;i150;O0; xxHash `- fast non`-cryptographic hash `- now part 
of Core.&]
[s5;l160;i150;O0; SHA256 code now part of the Core.&]
[s5;l160;i150;O0; plugin/pcre updated to the latest pcre version.&]
[s5;l160;i150;O0; plugin/zstd now provides [^http`:`/`/facebook`.github`.io`/zstd`/^ zs
td] compression (with multithreaded option).&]
[s5;l160;i150;O0; plugin/lz4 now can multithreaded.&]
[s22; GUI programming `& graphics&]
[s5;l160;i150;O0; PdfDraw and RichText now support PDF signatures.&]
[s5;l160;i150;O0; RichText now can change header/footer within document.&]
[s5;l160;i150;O0; ArrayCtrl got SetLineColor and GetCellRect methods.&]
[s22; TheIDE&]
[s5;l160;i150;O0; Comments now can be spell checked and there is 
comment word wrap and reformatting operation.&]
[s5;l160;i150;O0; New functions / icons that search the web (launch 
the browser) for text selected or current identifier.&]
[s5;l160;i150;O0; Optimal compilation mode is removed as confusing, 
it is now always either debug or release mode, release mode being 
configured for speed.&]
[s5;l160;i150;O0; When comparing files, differences within single 
lines are now shown.&]
[s5;l160;i150;O0; Most U`+`+ designers now support drag`&drop to 
reorganize lists.&]
[s5;l160;i150;O0; Find File window was remastered.&]
[s5;l160;i150;O0; New dialog for creating package file.&]
[s5;l160;i150;O0; Android application can be directly launch on emulator 
from TheIDE.&]
[s5;l160;i150;O0; Clang is now default compiler for native Android 
applications.&]
[s5;l160;i150;O0; C`+`+14 is now default standard for all Android 
builds.&]
[s5;l160;i150;O0; Various android builder improvements.&]
[s22; ESC&]
[s5;l160;i150;O0; New math functions like sin, cos, tg, pow etc.&]
[s5;l160;i150;O0; New replace in string function.&]
[s22; General&]
[s5;l160;i150;O0; Significantly improved the process of building 
TheIDE and UMK from tarball (POSIX environment). Compilation 
on multiple cores is possible with `"`-j`" option.&]
[s5;l160;i150;O0; Documentation improvements.&]
[s5;l160;i150;O0; Bug fixes in various areas.&]
[s24; &]
[s24; Current release&]
[s3; 2015.2 (rev 9251)[3  (Dec 2015)]&]
[s5; The main focus of this release was [^topic`:`/`/ide`/app`/Assist`$en`-us^ C`+`+ 
parser and Assist`+`+ features]. and [^topic`:`/`/ide`/app`/AndroidBuilder`$en`-us^ A
ndroid applications builder] in TheIDE (library does not yet 
support Android though).&]
[s22; Core&]
[s5;l160;i150;O0; Improved [* C`+`+11] support.&]
[s5;l160;i150;O0; Leap second of 2015 added to time routines.&]
[s22; GUI programming `& graphics&]
[s5;l160;i150;O0; Improved support of UHD displays.&]
[s5;l160;i150;O0; New QTF command [*@5 `{`{`*] is shortcut for [*@5 `{`{`~0/0] 
to simplify using invisible tables for organizing text.&]
[s5;l160;i150;O0; PdfDraw now supports urls (e.g. when converting 
QTF/RichText to PDF).&]
[s5;l160;i150;O0; RichText/QTF now support round borders for table 
cells.&]
[s5;l160;i150;O0; ScatterCtrl: new features.&]
[s22; TheIDE&]
[s5;l160;i150;O0; Assist`+`+ and C`+`+ parser now support C`+`+11 
and non`-project headers, parsing ability is generally improved.&]
[s5;l160;i150;O0; Android builder.&]
[s5;l160;i150;O0; UTF16 support, UTF BOM autodetection.&]
[s5;l160;i150;O0; Rename/Delete package functions.&]
[s5;l160;i150;O0; Layout designer has new code generation features 
and can jump to C`+`+ using the layout.&]
[s5;l160;i150;O0; Editor now [* truncates ]files longer than[*  200MB 
/ 1GB] (32/64 bits ide) while loading, makes them read`-only.&]
[s5;l160;i150;O0; Editor now shows [* misplaced whitespaces] in source 
files.&]
[s5;l160;i150;O0; TheIDE now detects binary files and provides binary 
viewer.&]
[s5;l160;i150;O0; Toolbar has new navigation icons and icons to switch 
editation mode (text/designer/binary).&]
[s5;l160;i150;O0; Legacy [* GDB]`-backended [* debugger ]was refurbished 
and became `'Standard`' debugger for GCC.&]
[s5;l160;i150;O0; Icon designer now shows images as files icons when 
inserting image files.&]
[s22; Win32 releases&]
[s5;l160;i150;O0; Win32 now does not come as .exe installer, but 
simple .7z archive, which acts as `"[* portable]`" installation. 
Nothing is written to the registry, nothing needs to be installed, 
simply running `"theide.exe`" setups everything needed. (theide.exe 
is 64bit executable. For those unlucky to still run 32`-bit os, 
there is theide32.exe).&]
[s5;l160;i150;O0; There is once again `'[* mingw]`' variant which is 
coupled with TDM64 release of mingw`-w64.&]
[s5; &]
[s3;H4; Archival releases:&]
[s5; If you are looking for informations about archival releases 
`- you can find them on the following [^topic`:`/`/uppweb`/www`/RoadmapHistorical`$en`-us^ s
ite].]]