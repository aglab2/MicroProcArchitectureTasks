all:
	wcl386 -bt=dos -l=dos4g lab1.c
	wcl386 -bt=dos -l=dos4g lab2.c
	wcl386 -bt=dos -l=dos4g lab3.c
	wcl386 -bt=dos -l=dos4g lab4.c
	sudo cp lab1.exe ../floppy/PROGS
	sudo cp lab2.exe ../floppy/PROGS
	sudo cp lab3.exe ../floppy/PROGS
	sudo cp lab4.exe ../floppy/PROGS
	sync

clean:
	rm -r *.err *.exe *.o
