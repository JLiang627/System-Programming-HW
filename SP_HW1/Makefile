#Exercise 1~5

all: E1 E2 E3 E4 E5


E1:
	@echo "Authors of the bash shell:"
	@man bash | grep -A5 AUTHORS
	@echo ""

E2:
	@echo "Return type of the read() system call:"
	@man 2 read | grep 'read(' | sed -n '2p'
	@echo ""

E3:	
	@echo "Header files of _exit():"
	@man _exit | grep "#include" | sed -n '1p'
	@echo ""
	
	@echo "Header files of setuid():"
	@man setuid | grep "#include"
	@echo ""
	
	@echo "Header files of fstat():"
	@man fstat | grep "#include" | sed -n '1p'
	@echo ""

E4:
	@echo "The shortest relative pathnaem of the file"
	@echo "../../../lib32/libc.so.6"
	@echo""

E5:
	@echo "The command being use to print the creation date of a file:"
	@echo "stat"




