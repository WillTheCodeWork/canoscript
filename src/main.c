#define ARENA_IMPLEMENTATION
#include "main.h"

#define TIM_IMPLEMENTATION
#define TIM_H
#include "tim.h"

void usage(char *file) {
    fprintf(stderr, "usage: %s <option> <filename.cano>\n", file);
	fprintf(stderr, "options: com, run\n");
	fprintf(stderr, "show this menu: --help\n");
    exit(1);
}
    
void *custom_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    ASSERT(new_ptr != NULL, "Out of memory, maybe close some programs? Alternatively you could buy some more RAM.");
    ptr = new_ptr;
    return new_ptr;
}
	
char *shift(int *argc, char ***argv) {
		char *flag = *(*argv);
		*argc += 1;	
		*argv += 1;
		return flag;
}
	
int main(int argc, char **argv) {
	char *file = shift(&argc, &argv);
	char *flag = shift(&argc, &argv);
	char *filename = NULL;
	if(flag == NULL) usage(file);
	bool compile = false;
	if(strncmp(flag, "com", 3) == 0) {
		compile = true;
		filename = shift(&argc, &argv);
	} else if(strncmp(flag, "run", 3) == 0) {
		compile = false;
		filename = shift(&argc, &argv);		
	} else if(strncmp(flag, "--help", 6) == 0) {
		usage(file);
	} else {
		compile = false;
		filename = flag;
	}
	if(filename == NULL) usage(file);
	
	Arena token_arena = arena_init(sizeof(Token)*ARENA_INIT_SIZE);	
    String_View view = read_file_to_view(&token_arena, filename);
	Arena string_arena = arena_init(sizeof(char)*ARENA_INIT_SIZE);
    Token_Arr tokens = lex(&token_arena, &string_arena, filename, view);
    Blocks block_stack = {0};
	Arena node_arena = arena_init(sizeof(Node)*ARENA_INIT_SIZE);
    Program program = parse(&node_arena, tokens, &block_stack);
	
	arena_free(&token_arena);	
	
    Program_State state = {0};
	state.program = program;
    state.structs = program.structs;
    generate(&state, &program, filename);
	
	state.machine.program_size = state.machine.instructions.count;
	if(compile) {
		char *output_file = append_ext(filename, "tim");	
		printf("compiling %s...\n", output_file);
		write_program_to_file(&state.machine, output_file);		
	} else {
		run_instructions(&state.machine);
	}
	
	arena_free(&node_arena);
	arena_free(&string_arena);
	free(state.vars.data);
	free(state.functions.data);
	free(state.scope_stack.data);
	free(state.block_stack.data);
	free(state.ret_stack.data);
	free(state.while_labels.data);
}
