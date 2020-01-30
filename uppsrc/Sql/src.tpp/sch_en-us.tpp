topic "";
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[l288;i1121;b17;O9;~~~.1408;2 $$7,0#10431211400427159095818037425705:param]
[i448;b42;O9;2 $$8,8#61672508125594000341940100500538:tparam]
[b42;2 $$9,9#13035079074754324216151401829390:normal]
[2 $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@(113.42.0) [s0;%% [*@7;4 .sch schema files]]}}&]
[s0;%% &]
[s0; [3 SQLID]&]
[s0; [3 ARRAY]&]
[s0;3 &]
[ {{4746:5254^ [s0; [* TABLE(][*/ table][* )]]
:: [s0; Defines a new table. This creates S`_ structures to map SQL 
table to C`+`+ structure, introspection records and also can 
create SQL schema creation/upgrade scripts.]
:: [s0; [* TABLE`_I(][*/ table][* , ][*/ base1][* )]]
:: [s0; Defines a new table which also has all columns of [/ base1] which 
can be either table or type.]
:: [s0; [* TABLE`_II(][*/ table][* , ][*/ base1][* , ][*/ base2][* )]]
:: [s0; Defines a new table which also has all columns of [/ base1 ]and 
[/ base2 ]which can be either tables or types.]
:: [s0; [* TABLE`_III(][*/ table][* , ][*/ base1][* , ][*/ base2][* , ][*/ base3][* )]]
:: [s0; Defines a new table which also has all columns of [/ base1 , base2 
]and [/ base3] which can be either tables or types.]
:: [s0; [* END`_TABLE]]
:: [s0; Ends Table definition.]
:: [s0; [* TYPE(][*/ type][* )]]
:: [s0; Defines a new type. Unlike TABLE, TYPE does not produce table 
creation code in schema sql scripts. Following this header is 
a list of columns and inline attributes, ending with END`_TYPE.]
:: [s0; [* TYPE`_I(][*/ type][* , ][*/ base][* )]]
:: [s0; Defines a new type which also has all columns of [/ base.]]
:: [s0; [* TYPE`_II(][*/ type][* , ][*/ base1][* , ][*/ base2][* )]]
:: [s0; Defines a new type which also has all columns of [/ base1 ]and 
[/ base2].]
:: [s0; [* TYPE`_III(][*/ type][* , ][*/ base1][* , ][*/ base2][* , ][*/ base3][* )]]
:: [s0; Defines a new type which also has all columns of [/ base1 , base2 
]and [/ base3].]
:: [s0; [* END`_TYPE]]
:: [s0; Ends type definition.]
:: [s0; [* SQL`_NAME(][*/ id][* )]]
:: [s0; Provides an alternate SQL name of previous item. When used, 
the column name will be used in C`+`+ context, while this alternate 
name will be use in SQL context. [*/ id] is C literal.]
:: [s0; ]
:: [s0; ]
:: [s0; [* INT(][*/ column`_name][* )]]
:: [s0; Column capable of storing 32`-bit integer value, int type in 
C`+`+.]
:: [s0; [* INT`_ARRAY(][*/ column`_name][* , ][*/ items][* )]]
:: [s0; Array of 32`-bit integers.]
:: [s0; [* INT64(][*/ column`_name][* )]]
:: [s0; Column capable of storing 64`-bit integer value, int64 type 
in C`+`+. `[sqlite3`]]
:: [s0; [* INT64`_ARRAY(][*/ column`_name][* , ][*/ items][* )]]
:: [s0; Array of 64`-bit integers. `[sqlite3`]]
:: [s0; [* DOUBLE(][*/ column`_name][* )]]
:: [s0; Column capable of storing double precision floating point value, 
double type in C`+`+.]
:: [s0; [* DOUBLE`_ARRAY(][*/ column`_name][* , ][*/ items][* )]]
:: [s0; Array of double values.]
:: [s0; [* STRING(][*/ column`_name][* , ][*/ maxlen][* )]]
:: [s0; Column capable of storing string with character count limit 
[/ maxlen], String type in C`+`+.]
:: [s0; [* STRING`_ARRAY(][*/ column`_name][* , n, items)]]
:: [s0; Array of strings.]
:: [s0; [* DATE(][*/ column`_name][* )]]
:: [s0; Column capable of storing calendar date (without time), Date 
type in C`+`+.]
:: [s0; [* DATE`_ARRAY(][*/ column`_name][* , items)]]
:: [s0; Array of dates.]
:: [s0; [* TIME(][*/ column`_name][* )]]
:: [s0; Column capable of storing calendar date with time, Time type 
in C`+`+.]
:: [s0; [* TIME`_ARRAY(][*/ column`_name][* , items)]]
:: [s0; Array of date`-times.]
:: [s0; [* BOOL(][*/ column`_name][* )]]
:: [s0; Column capable of storing boolean value, bool type in C`+`+. 
Important: for compatibility reasons between various SQL engines, 
BOOL is always emulated with single character text value (`"1`" 
for true). Conversion is provided for S`_ types, but SQL commands 
must account for this. ]
:: [s0; [* BOOL`_ARRAY(][*/ column`_name][* , items)]]
:: [s0; Array of bools.]
:: [s0;* ]
:: [s0; ]
:: [s0; [* BLOB(][*/ column`_name][* )]]
:: [s0; Binary data of unlimited size. `[sqlite3`]]
:: [s0;* ]
:: [s0; ]
:: [s0; [* NOT`_NULL]]
:: [s0; Not null constraint.]
:: [s0; [* PRIMARY`_KEY]]
:: [s0; Primary key column.]
:: [s0; [* AUTO`_INCREMENT]]
:: [s0; Autoincrement column, this gets increasing values when inserted.]
:: [s0; [* REFERENCES(][*/ table][* )]]
:: [s0; Foreign key specification for column.]
:: [s0; [* REFERENCES`_CASCADE(][*/ table][* )]]
:: [s0; Foreign key with `"ON DELETE CASCADE`" option. `[sqlite3`]]
:: [s0; [* DUAL`_PRIMARY`_KEY(][*/ column1][* , ][*/ column2][* )]]
:: [s0; Dual primary key for table.]
:: [s0; [* SQLDEFAULT(v)]]
:: [s0; Default value for column.]
:: [s0; [* INDEX]]
:: [s0; Column has index.]
:: [s0; [* UNIQUE]]
:: [s0; Column has UNIQUE constraint.]
:: [s0; [* UNIQUE`_LIST(][*/ constraint`_name][* , ][*/ list][* )]]
:: [s0; Creates UNIQUE constraint for set of columns. [*/ list] is a C 
literal with comma separated column names.]
:: [s0; [* DUAL`_UNIQUE(][*/ column1][* , ][*/ column1][* )]]
:: [s0; UNIQUE constraint for two columns.]
:: [s0; [* INDEX`_LIST(][*/ constraint`_name][* , ][*/ list][* )]]
:: [s0; [*/ list] is a C literal with comma separated column names.]
:: [s0; [* SQLCHECK(][*/ constraint`_name][* , ][*/ check][* )]]
:: [s0; Adds CHECK constraint, [*/ check] is C literal with check expression.]}}&]
[s0;%% &]
[s0;3 &]
[s0;3 &]
[s0;%% ]]