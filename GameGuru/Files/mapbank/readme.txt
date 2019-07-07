MAPBANK Folder
==============

FPM level files go in this folder as the default location for all saved levels.

Manual Entity Replacement System
--------------------------------

If your FPM relies on entities that have since moved location, there is a manual
method you can use to correct the entity references. Missing entities are reported
when you load an FPM file inside the .LOG file that accompanies the FPM file. To
replace entity references, follow the steps below:

1. Rename the .LOG file to .REPLACE (i.e. mylevel.log becomes mylevel.replace)
2. Open the new .REPLACE file in NOTEPAD
3. Ensure the top line containing "MISSING MEDIA:" remains as the first line
4. Search for all occurances of '=replace' and delete (i.e. entitybank\\item.fpe=replaceentitybank\\item.fpe becomes entitybank\\item.fpe=entitybank\\item.fpe)
5. Edit the right side of the '=' symbol to enter a new reference for the specific entity
6. Ensure there is a blank line at the end of the file (the last line)
7. When all references edited in, save the file and exit NOTEPAD
8. Now launch the software and load in the FPM that contained missing entities
9. When the level loads in fine and does not report any missing entities, save the FPM
10. If the level reports missing assets, do NOT save the FPM, but repeat from step [2]
11. When the new FPM with amended entities works fine, delete the .REPLACE file

