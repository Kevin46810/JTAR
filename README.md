Relevant File class operations:
-	getSize(): retrieves size of file
-	getName(): retrieves name of file
-	getPmode(): retrieves pmode of file
-	getStamp(): retrieves access time of file
-	flagAsDir(): labels a file as a directory
-	isADir(): checks if a file is a directory
-	Explicit constructor containing name, pmode, size, and access time
-	Default constructor

Plan for jtar:
1.	cf:
  a.	From a list of entered filenames, loop through it and make a separate method that extracts their pmodes, sizes, and stamps and stores them in variables. Also create a list that stores File objects
  b.	Using those variables, apply the File explicit constructor to make an object
  c.	If the file is a directory, use flagAsDir() and put in the list of File objects. After that, enter a recursive method to extract its contents
  d.	If it’s a file, just put in the File object list
  e.	Once the list is filled, loop through and call a writeToTar method with each iteration to fill the tarfile
2.	xf:
  -	Read through the tarfile in a loop to systematically retrieve each object stored:
    i.	If it’s a file:
      I.	Create an output file from the File getName() method
      II.	Loop through each individual character stored in the tarfile and write it into the new output file
      III.	Restore the original pmode and access time with system calls using getPmode() and getStamp()
    ii.	If it’s a directory:
      I.	Check that the directory exists
      II.	If it doesn’t, then do system calls of mkdir getName() and cd getName()
      III.	Do two more system calls to restore the original pmode and access time for the directory
      IV.	Make a recursive call to restore the directory’s contents in the same way
