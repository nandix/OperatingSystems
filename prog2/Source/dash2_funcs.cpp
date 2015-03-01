

string run_sys_call( string command )
{
	// Karlsson's variables pasted in!
	char *line; 
	char *args[100]; 
	int   num_args; 
	int   childpid; 
	int   status; 
	struct rusage usage;

	// Convert to a c-style string because Karlsson is into that
	//	sort of stuff... and it means I don't need to modify the
	//	code hardly at all. :)
	line = (char*)command.c_str();

	num_args = 0; 
	args[num_args] = strtok(line, " "); 
	while (args[num_args] != NULL) 
	{ 
		num_args++; 
		args[num_args] = strtok(NULL, " "); 
	} 
	num_args--; 

	printf("\n"); 
	childpid = fork(); 
	if (childpid == 0) 
	{ 
		printf( "Child pid: %d\n", getpid() );
		execvp(args[0], args); 
		perror("Exec failed: "); 
		exit(5); 
	} 
	
	wait(&status); 

	printf("\n");
	getrusage( RUSAGE_CHILDREN, &usage);

	printf("User Time: %ld.%ld\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec ); 							
    printf("System Time: %ld.%ld\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec ); 							      			
    printf("Soft Page Faults: %ld\n", usage.ru_minflt);     		
    printf("Hard Page Faults: %ld\n", usage.ru_majflt);     		
    printf("Swaps: %ld\n", usage.ru_nswap);       					



	return "\n    STUFFHAPPENED\n";
} 
