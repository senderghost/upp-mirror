TABLE_(TESTPARTNER)
   SERIAL_  (TESTPARTNER_ID) PRIMARY_KEY
   STRING_  (TESTPARTNER_NAME, 200) INDEX
   STRING_  (TESTPARTNER_ADDRESS, 200)
END_TABLE

TABLE_(TESTPRODUCT)
   SERIAL_  (TESTPRODUCT_ID) PRIMARY_KEY
   STRING_  (TESTPRODUCT_NAME, 200) INDEX
   STRING_  (TESTPRODUCT_UNIT, 20)
END_TABLE


