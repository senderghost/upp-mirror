TABLE_(TABLE1)
   INT_     (ID) PRIMARY_KEY
   STRING_  (NAME, 200)
   STRING_  (LASTNAME, 200)
   INT_     (BDATE)
END_TABLE

TABLE_(TABLE2)
   INT      (ID) PRIMARY_KEY
   INT_     (TABLE1_ID) REFERENCES(TABLE1)
   STRING   (NAME, 200)
   STRING   (LASTNAME, 200)
   INT      (BDATE)
END_TABLE
