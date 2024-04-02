Running the Script
---------------------------------------------------------------------------------------------
1. Navigate to Project Root (`Assignment`) using Terminal:
    cd path/to/Assignment 3
2. List Files to Confirm Directory:
    ls
    *Verify that you are in the correct directory.

3. Clean Old Compiled Files if any:
    make clean
    *Delete previously compiled `.o` files.

4. Compile Project Files:
    make
    *Compile all project files, including `test_assign3_1.c`.

5. Run `test_assign3_1.c` File:
    make run
    *Execute the `test_assign3_1.c` file.

6. Compile Test Expression Files:
    make test_expr
    *Compile test expression-related files, including `test_expr.c`.

7. Run `test_expr.c` File:
    make run_expr
    *Execute the `test_expr.c` file.



1. TABLE AND RECORD MANAGER FUNCTIONS
------------------------------------------------------------------------------------
1. `initialize_RecordManager(void *managementData)`
 Description: Initializes the Record Manager module.
 Parameters: 
  - `managementData`: Pointer to management data.
 Return Value: Returns `RC_OK` upon successful initialization.

2. `shutdown_RecordManager()`
- Description: Shuts down the Record Manager module and deallocates associated resources.
- Return Value: Returns `RC_OK` upon successful shutdown.

 3. `create_Table(char *tableName, Schema *tableSchema)`
- Description: Creates a table with the specified name and schema.
- Parameters:
  - `tableName`: Name of the table to be created.
  - `tableSchema`: Pointer to the schema of the table.
- Return Value: Returns `RC_OK` upon successful creation of the table.

 4. `open_Table(RM_TableData *tableData, char *tableName)`
- Description: Opens a table with the specified name.
- Parameters:
  - `tableData`: Pointer to RM_TableData structure to store table information.
  - `tableName`: Name of the table to be opened.
- Return Value: Returns `RC_OK` upon successful opening of the table.

 5. `close_Table(RM_TableData *tableData)`
- Description: Closes the table referenced by `tableData`.
- Parameters:
  - `tableData`: Pointer to RM_TableData structure representing the table to be closed.
-Return Value:Returns `RC_OK` upon successful closing of the table.

# Usage
1. Call `initialize_RecordManager()` to initialize the Record Manager module before using any other functions.
2. Create a table using `create_Table()` by providing the table name and schema.
3. Open a table using `open_Table()` to perform operations on it.
4. After finishing operations, close the table using `close_Table()` to release resources.

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
2. createSchema
Description:Creates a schema object with specified attributes, data types, and keys.
Procedure:
Allocate memory space for a `Schema` structure using the `malloc` function, ensuring space equal to the size of a `Schema` structure.
Set the `numAttr` field of the schema to the provided `numAttr`.
Set the `attrNames` field of the schema to the provided `attrNames`.
Specify the data types for the attributes in the schema using the provided `dataTypes`.
Define the length of data types, especially for strings, in the schema using the provided `typeLength`.
Determine the size of the key in the schema using the provided `keySize`.
Define the attributes that constitute the key in the schema using the provided `keys`.
Return the created schema.

3. freeSchema
Description:Removes a schema from memory and releases the occupied memory space.
Procedure:
Deallocate the memory space occupied by the `schema` variable using the `free` function.
Return a success status code (`RC_OK`) indicating that the operation was completed successfully.

5. ATTRIBUTE FUNCTIONS
--------------------------------------------------------------------------------------

1. createRecord
Description:Creates a new record based on the provided schema.
Procedure:
Allocate memory space for a new record using `malloc` function, ensuring space equal to the size of a `Record` structure.
Calculate the size of the record based on the provided schema using the `getRecordSize` function.
Allocate memory space for the data of the new record based on the calculated record size.
Initialize the page and slot position of the new record as -1 to indicate it's a new record with an unknown position.
Set the first character of the record's data to '-' as part of a Tombstone mechanism indicating that the record is empty.
Append '\0' (NULL character) to the record's data after the Tombstone character.
Assign the newly created record to the pointer passed as an argument.
Return an OK status to indicate that the record creation was successful.

2. attrOffset
Description:Sets the offset (in bytes) from the initial position to the specified attribute of the record into the 'result' parameter.
Procedure:
Initialize a counter variable `i` to 0 and set the `result` parameter to 1.
Iterate through each attribute in the schema until reaching the specified attribute number `attrNum`.
For each attribute, check its data type using `if-else` statements:
   - If the attribute is of type STRING, add its defined length (`typeLength`) to the `result`.
   - If the attribute is of type INTEGER, add the size of an integer (`sizeof(int)`) to the `result`.
   - If the attribute is of type FLOAT, add the size of a float (`sizeof(float)`) to the `result`.
   - If the attribute is of type BOOLEAN, add the size of a boolean (`sizeof(bool)`) to the `result`.
Increment the counter `i` to move to the next attribute in the schema.
Once all attributes up to the specified one have been processed, return RC_OK.

3. freeRecord
Description:Deallocates the memory used by a record.
Procedure:
Free the memory space previously allocated to the record using the `free` function.
Return the status code indicating successful execution.

4. getAttr
Description:Retrieves an attribute from the given record in the specified schema.
Procedure:
Calculate the offset value of attributes based on the attribute number using the `attrOffset` function.
Allocate memory space for the `Value` data structure where the attribute values will be stored.
Determine the starting position of the record's data in memory.
Add the calculated offset to the starting position.
Retrieve the attribute's value depending on its data type:
   - For STRING attributes, copy the string from the location pointed by `dataPointer` and append '\0' to denote the end of the string in C.
   - For INTEGER attributes, copy the integer value from `dataPointer`.
   - For FLOAT attributes, copy the float value from `dataPointer`.
   - For BOOLEAN attributes, copy the boolean value from `dataPointer`.
Set the pointer to the attribute value.
Return RC_OK.

5. setAttr
Description:Sets the attribute value in the record in the specified schema.
Procedure:
Calculate the offset value based on the attribute number using the `attrOffset` function.
Get the starting position of the record's data in memory.
Add the offset to the starting position.
Check the data type of the attribute and set its value accordingly:
   - For STRING attributes, copy the attribute's value to the location pointed by the record's data.
   - For INTEGER attributes, set the attribute value.
   - For FLOAT attributes, set the attribute value.
   - For BOOLEAN attributes, set the attribute value.
Return RC_OK.
