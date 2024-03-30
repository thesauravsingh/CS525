1. TABLE AND RECORD MANAGER FUNCTIONS
------------------------------------------------------------------------------------
6. deleteTable:
Description: Deletes the table with the specified name.
Procedure:
Removes the page file associated with the table from memory using the storage manager.

7. getNumTuples:
Description: Returns the number of tuples (records) in the table.
Procedure:
Accesses the tuples count from the data structure and returns it.

2. RECORD FUNCTIONS
-------------------------------------------------------------------------------------
1. insertRecord:
Description: Inserts a new record into the table at the specified location.
Procedure:
Pins the page where the record will be inserted.
Finds an available slot within the page.
Marks the page as dirty after insertion.
Unpins the page.

2. deleteRecord:
Description: Deletes a record from the table at the specified location.
Procedure:
Pins the page containing the record to be deleted.
Marks the page as dirty after deletion.
Unpins the page.

3. updateRecord:
Description: Updates an existing record in the table at the specified location.
Procedure:
Pins the page containing the record to be updated.
Marks the page as dirty after update.
Unpins the page.

3. SCAN FUNCTIONS
--------------------------------------------------------------------------------------


4. SCHEMA FUNCTIONS
--------------------------------------------------------------------------------------


5. ATTRIBUTE FUNCTIONS
--------------------------------------------------------------------------------------
