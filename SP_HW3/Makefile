all:c_tail newfile exe

c_tail:SP_HW_3.c
	gcc SP_HW_3.c -o c_tail

newfile:
	@touch script.txt
	@echo "this is the first line\nThe file is for homework test\nthis is the third line\nthe command should be c_tail -n num filename\nthe fiveth line is the last one" >> script.txt

exe:
	@echo '------------------------------------------------'
	@echo 'starting execute the program'
	./c_tail -n 3 script.txt

clean:
	rm c_tail script.txt
	
