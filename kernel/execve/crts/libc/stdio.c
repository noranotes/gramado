/*
 * File: uitm\stdio.c 
 *
 * Descri��o:
 *     +Rotinas de input/output padr�o.
 *     +Algumas rotinas de console.
 *
 * Ambiente:
 *     Apenas kernel mode.
 *
 *
 * @todo: Buffering and pipes.
 *        Priorizar opera��es com disco.
 * 
 * Credits:
 *     printf support - Georges Menie's tutorial.
 *     https://www.menie.org/georges/embedded/small_printf_source_code.html
 *     
 * History:
 *     2015 - Create by Fred Nora.
 *
 */
  
 
#include <kernel.h>


//
// Fun��es importadas.
//

extern void background();  //Assembly.
extern void refresh_screen();


/*
 *********************************************
 * fclose:
 *     Close a file.    
 */
int fclose (FILE *stream){
	
	if( (void *) stream != NULL ){
		stream = NULL;
	};		

	//
	// @todo: Implementar.
	//


//done:
	
	return (int) 0;
};


/*
 ********************************************************************
 * fopen:
 *     @field 2
 *     Open a file.
 *
 *     #bugbug: 
 *     Por enquanto o limite do tamanho do arquivo � 4KB.
 *     
 *     #todo: 
 *     Mas j� temos recursos para abrimos arquivos maiores 
 *     usando essa fun��o. � s� obtermos o tamanho do arquivo 
 *     e alocarmos um buffer do tamanho necess�rio
 */
FILE *fopen ( const char *filename, const char *mode ){
	
    unsigned long fileret;
	struct _iobuf *stream;
	
	//Buffer usado cara colocar a estrutura.
    unsigned char struct_buffer[1024];
	
	//Buffer para armazenar o arquivo que vamos abrir.
	unsigned char *file_buffer;		
	
	//buffer da estrutura.
	stream = (FILE *) &struct_buffer[0];		


	/*
	 *usado para alocar mais p�ginas.
	Ret = (void*) allocPageFrames(2);      //8KB. para imagem pequena.
	if( (void*) Ret == NULL ){
	    printf("Ret fail\n");
        goto fail;	
        //return NULL;		
	}
    */	
	
	//alocando apenas uma p�gina.
	//4KB
	//Buffer do arquivo.
	//@todo: Deve ser maior. Do tamanho do arquivo.
	//Podemos usar outra rotina de aloca��o de p�gina.
	
	//#todo:
	//j� temos recursos para alocar mem�ria para um buffer maior.
	//obs: Essa aloca��o vai depender do tamanho do arquivo.
	
	file_buffer = (unsigned char *) newPage();
	
	if ( (unsigned char *) file_buffer == NULL )
	{
		printf("fopen: file_buffer");
		die();
	}	
	  
	stream->_base = &file_buffer[0];
	stream->_ptr = stdin->_base;
	stream->_cnt = PROMPT_MAX_DEFAULT;
	stream->_file = 0;
	stream->_tmpfname = (char *) filename;	
	
	// Loading file.
//loadingFile:

	fileret = fsLoadFile ( (unsigned char *) stream->_tmpfname, 
	            (unsigned long) stream->_base );
	
	if ( fileret != 0 )
	{	
		printf("fopen: fsLoadFile fail\n");
		
		stream = NULL;
		
		return NULL;
	};
	
	//...
	
//done:

	return (FILE *) stream; 	
};


/*
 ************************************************************************
 * scroll:
 *     Isso pode ser �til em full screen e na inicializa��o do kernel.
 *
 * *Importante: Um (ret�ngulo) num terminal deve ser o lugar onde o buffer 
 * de linhas deve ser pintado. Obs: Esse ret�ngulo pode ser configurado atrav�s 
 * de uma fun��o.
 *     Scroll the screen in text mode.
 *     Scroll the screen in graphical mode.
 *     @todo Poderiam ser duas fun��es: ex: gui_scroll(). 
 *    
 * * IMPORTANTE
 *   O que devemos fazer � reordenar as linhas nos buffers de linhas
 *     para mensagens de texto em algum terminal.
 * 
 * @todo: Ele n�o ser� feito dessa forma, termos uma disciplica de linhas
 * num array de linhas que pertence � uma janela.
 *
 * @todo: Fazer o scroll somente no stream stdin e depois mostrar ele pronto.
 *
 */
void scroll (void){
	
    unsigned short i;
    unsigned short j;


	//indice para o backbuffer.
	int Start, End;
	int Source, Dest;
	
	// ponteiros usados no modo gr�fico.
	unsigned char *backbuffer = (unsigned char *) BACKBUFFER_BASE;
	
	// ponteiros usados no modo texto.
	
	//In�cio da tela e in�cio da segunda linha.
	unsigned short *p1 = (unsigned short *) SCREEN_START;
    unsigned short *p2 = (unsigned short *) (SCREEN_START + 2 * SCREEN_WIDTH);

	//salvar cursor.
	unsigned long OldX, OldY;
	
	
	//
	// Se estamos em Modo gr�fico (GUI).
    //

//graphics_mode:
	
	if ( VideoBlock.useGui == 1 )
	{
	
		//Modificar o backbuffer. BACKBUFFER_BASE
        
		//mover o backbuffer come�ando da segunda linha.
		//exclui a primeira linha e a segunda linha ser� a nova primeira.
		
		//so que na verdade isso deve ser feito apenas para a 
		//janela onde o texto est� e n�o para o backbuffer inteiro.
		//por enquanto fica assim.
		
		//#bugbug: 
		//Valores determinados para width e height.
		//Precisamos pegar valores salvos.
		
		Start = 0;
		Source = Start+(3*800*8);  //Come�a da segunda linha.
		Dest = 0;                  //destino o in�cio do backbuffer.
		
		End = (3*800)*600;       //termina ao fim de 600 linhas.
		
		//copy. 
		//Contagem do total a ser transferido.
		for ( Start=0; Start < End; Start++ ){
			
            backbuffer[Start] = backbuffer[Source];
            Source++;    			
		};
		
        //Limpa a �ltima linha.
		
		//salva cursor
		OldX = g_cursor_x;
		OldY = g_cursor_y;
		
		//cursor na ultima linha.
		//g_cursor_x = 0;
		g_cursor_x = g_cursor_left;
		g_cursor_y = (g_cursor_bottom-1);
		
		for ( i = g_cursor_x; i < g_cursor_right; i++ ){
		    _outbyte(' ');    	
		};
	
		// Reposiciona o cursor na �ltima linha.	
		
		g_cursor_x = g_cursor_left;
		g_cursor_y = OldY;
		
		refresh_screen();
		goto done;
	};

	// Se estavermos em modo texto.
	
//text_mode:
	
	if ( VideoBlock.useGui == 0 )	
	{
	    //24 vezes.
        for ( i=0; i < (SCREEN_HEIGHT -1); i++ )
	    {
	        //80 vezes
            for ( j=0; j < SCREEN_WIDTH; j++ ){
                *p1++ = *p2++;
		    };
			//Nothing.
		};
	
	    //80 vezes.
        for ( i=0; i < SCREEN_WIDTH; i++ ){
		    *p1++ = 0x07*256 + ' '; 
	    };
		
		//Nothing.
		goto done;
    };
	
    //Nothing.	
	
done:
	return;
};


/*
 * kclear:
 *     Limpa a tela em text mode.
 *     # isso n�o faz parte da lib c, mudar para 
 *     o field 3.
 */
int kclear(int color) {
	
    //fis=0x000B8000, virtual=0x800000;
	char *vm = (char *) g_current_vm;  
	
    unsigned int i = 0;
	
//Graphic Mode.
//gui_mode:
	if ( VideoBlock.useGui == 1 ){
	    backgroundDraw(COLOR_BLACK);       
		return (int) 0; 		
	}
	
//Text Mode.
//text_mode:	
	while ( i < (SCREEN_WIDTH*SCREEN_HEIGHT*2) ) 
    { 
        vm[i] = 219; 
        i++; 
        
        vm[i] = color; 
        i++; 
    };
	
	//Cursor.
	g_cursor_x = 0;
	g_cursor_y = 0;
	
//done.
done: 
    return (int) 0; 
}; 


/*
 * kclearClientArea:
 *     Limpa a �rea de cliente em text mode.
 *     Limpa 24 das 25 linhas.
 *     N�o limpa a primeira de cima.
 */
int kclearClientArea(int color)
{
	char *vm = (char *) g_current_vm;    //fis=0x000B8000; 	
    unsigned int i = (SCREEN_WIDTH*2);
      
    while(i < (SCREEN_WIDTH*SCREEN_HEIGHT*2)) 
    { 
        vm[i] = 219; 
        i++; 
        
        vm[i] = color; 
        i++; 
    };
	
	//Cursor.
	g_cursor_x = 0;
	g_cursor_y = 0;
    
done:
    return (int) 0; 
}; 


/*
 **************************************************************
 * kprint:
 *     Rotina simples de escrita em kernel mode.
 *     @todo: isso deve ir para outro lugar. uitm/libk 
 */
int kprint( char *message, unsigned int line, int color )
{ 
    char *vm = (char *) g_current_vm;    //fis=0xb8000, vir=0x800000; 
    unsigned int i; 

	i = (line*SCREEN_WIDTH*2); 

    while( *message != 0 ) 
    { 
        if( *message == '\n' )
        { 
            line++; 
            i = ( line*SCREEN_WIDTH*2 ); 
            message++; 
        }else{
			
            vm[i] = *message; 
            message++; 
            i++; 
            vm[i]= color; 
            i++; 
        }; 
    };
	
//done:	
    return (int) 0; 
}; 



/*
 *  ==== Segue aqui a fun��o printf ====
 */

/*
 *****************************
 * prints:
 *     Rotina de suporta a printf.
 */
static int prints ( char **out, const char *string, int width, int pad )
{
    register int pc = 0, padchar = ' ';

    if(width > 0) 
	{
	    register int len = 0;
		register const char *ptr;
		
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	};
	
	if( !(pad & PAD_RIGHT) ) 
	{
		for ( ; width > 0; --width){
		    printchar(out,padchar);
			++pc;
		};
	};
	
	for ( ; *string; ++string ){
		
		printchar(out, *string);
		++pc;
	};
	
	for ( ; width > 0; --width ){
		
		printchar(out,padchar);
		++pc;
	};

//done:	
	return (int) pc;
};


/*
 ****************************************
 * printi:
 *     Rotina de suporta a printf.
 *
 */
static int 
printi( char **out, 
        int i, 
		int b, 
		int sg, 
		int width, 
		int pad, 
		int letbase )
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if ( i == 0 ) 
	{
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	};
	
	if ( sg && b == 10 && i < 0 ){
		
		neg = 1;
		u = -i;
	}

	s = ( print_buf + ( PRINT_BUF_LEN -1 ) );
	
	*s = '\0';

	while (u) 
	{
		t = u % b;
		
		if ( t >= 10 )
		    t += letbase - '0' - 10;
		    *--s = t + '0';
		    u /= b;
	};

	if (neg) 
	{
		if ( width && (pad & PAD_ZERO) ) 
		{
		    printchar(out, '-');
			++pc;
			--width;
		}else {
			
			*--s = '-';
		};
	};

	// #obs: retorna pc + o retorno da fun��o.
	
	return (int) pc + prints(out, s, width, pad);
};


/*
 ****************************************
 * print:
 *     Rotina de suporta a printf.
 *
 */
static int print ( char **out, int *varg ){
	
	register int width, pad;
	register int pc = 0;
	register char *format = (char *) (*varg++);
	char scr[2];

	for( ; *format != 0; ++format ) 
	{
		if ( *format == '%' ) 
		{
			++format;
			width = pad = 0;
			
			if ( *format == '\0' ) 
			    break;
			
			if ( *format == '%' ) 
			    goto out;
			
			if ( *format == '-' ) 
			{
				++format;
				pad = PAD_RIGHT;
			};
			
			while ( *format == '0' ) 
			{
				++format;
				pad |= PAD_ZERO;
			};
			
			for ( ; *format >= '0' && *format <= '9'; ++format ){
				
				width *= 10;
				width += *format - '0';
			};
			
			if ( *format == 's' ) 
			{
				register char *s = *((char **)varg++);
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			
			if ( *format == 'd' ){
				pc += printi (out, *varg++, 10, 1, width, pad, 'a');
				continue;
			}
			
			if ( *format == 'x' ){
				pc += printi (out, *varg++, 16, 0, width, pad, 'a');
				continue;
			}
			
			if ( *format == 'X' ){
				pc += printi (out, *varg++, 16, 0, width, pad, 'A');
				continue;
			}
			
			if ( *format == 'u' ){
				pc += printi (out, *varg++, 10, 0, width, pad, 'a');
				continue;
			}
			
			if ( *format == 'c' ) 
	        {
				/* char are converted to int then pushed on the stack */
				scr[0] = *varg++;
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		
		}else{
			
		    out:
			    printchar (out, *format);
			    ++pc;
		};
	};
	
	if (out) 
        **out = '\0';
	
	return (int) pc;
};


/*
 ***************************************************
 * printf:
 *     @field 2
 *     The printf function.
 *     Assuming sizeof(void *) == sizeof(int).
 */
int printf ( const char *format, ... ){
    
    register int *varg = (int *) (&format);
	return (int) print (0,varg);
};


/*
 *************************************************
 * panic:
 *     # isso n�o faz parte da lib c, poderia ir para o @field 3.
 *     Kernel panic function.
 *     @todo: Esse fun��o deve retornar void.
 *     Essa fun��o faz parte da libC ??
 *     Essa rotina deveria ir para outro lugar.
 *     provavelmente em /sm
 */
void panic( const char *format, ... ){
	
	register int *varg = (int *)(&format);
	
	printf("crts-libc-stdio-panic: KERNEL PANIC\n");
		
    switch ( VideoBlock.useGui )
    {
		//text mode
		case 0:
	        //kclear(0);
	        print(0,varg);		
		    break;
		
		//graphics mode
		case 1:
	        //backgroundDraw(COLOR_BLACK);
		    //printf("crts-libc-stdio-panic: KERNEL PANIC\n");
		    print(0,varg);		
		    break;

        default:
	        //backgroundDraw(COLOR_BLACK);
		    //printf("crts-libc-stdio-panic: KERNEL PANIC\n");
		    print(0,varg);			
            break; 		
	};
    die();	
};


/*
 **********************************************************************
 * sprintf:
 *     @field 2
 *     int sprintf ( char * str, const char * format, ... );
 *     Composes a string with the same text that would be 
 * printed if format was used on printf, 
 * but instead of being printed, the content is stored 
 * as a C string in the buffer pointed by str.
*/
int sprintf ( char *str, const char *format, ... ){
		
    register int *varg = (int *) (&format);
	return (int) print (&str, varg);
};


/*
 ************************************************
 * fprintf:
 *     @field 2
 *
 */
int fprintf ( FILE *stream, const char *format, ... ){
	
    register int *varg = (int *) (&format);
	
	if ( (void*) stream != NULL )
	{
		//??
		//@todo Error. O erro deveria retornar 1.
        return 0;   		
	};		

	char *str = (char *) stream->_ptr;
	
//done:	
	return (int) print (&str, varg);	
};



/*
 *************************************************************
 * printchar:
 *     Coloca o caractere na string ou imprime.
 * Essa fun��o chama uma rotina que dever� tratar o caractere e em seguida 
 * envi�-lo para a tela.
 * Essa rotina � chamada pelas fun��es: /print/printi/prints.
 */
static void printchar (char **str, int c)
{
	if (str){
		
		**str = c;
		++(*str);
		
	}else (void) putchar (c);
};


/*
 ********************************************************************
 * putchar:
 *     Put a char on the screen. (libC).
 *     Essa rotina chama uma rotina de tratamento de caractes, somente
 * depois � que o caractere ser� enviado para a tela.
 *     Essa rotina � chamada pelas fun��es: /printchar/input/.
 */
int putchar (int ch){ 
   
    //Em cedge.c
    outbyte (ch);
    
	return (int) ch;    
};


/*
 * getchar:
 *    @todo isso deve er oferecido como servi�o pelo kernel.
 *
 * The getchar function is equivalent to getc with stdin as the value of the stream argument.
 */
/* 
int getchar()
{
	//terminal.h
	
	
	//erro
	if( teminalfeedCHStatus != 1 )
	{
		return (int) -1;
	}
	
	
done:
    teminalfeedCHStatus = 0;
	return (int) teminalfeedCH;
}
*/


/*
 essa fun��o � legal ... habilitar quando der.
 
void stdio_ClearToEndOfLine();
//limpa com caracteres em branco at� antes da posi��o do cursor.
void stdio_ClearToEndOfLine()
{
    unsigned u;
    unsigned long OldX, OldY;
    
    OldX = g_cursor_x;
	OldY = g_cursor_y;
	
	//de onde o cursor est� at� o fim da linha.
	for( u = g_cursor_x; u < g_cursor_right; u++ )
	{
       _outbyte(' ');
    }
	
    g_cursor_x = OldX;
	g_cursor_y = OldY;

}
*/


/*
 essa fun��o � legal ... habilitar quando der.
 
void stdio_ClearToStartOfLine();
//limpa com caracteres em branco at� antes da posi��o do cursor.
void stdio_ClearToStartOfLine()
{
    unsigned u;
    unsigned long OldX, OldY;
    
    OldX = g_cursor_x;
	OldY = g_cursor_y;
	
	//In�cio da linha.
    g_cursor_x = 0;
	g_cursor_y = OldY;	
	
	//de onde o cursor est� at� o fim da linha.
	for( u = g_cursor_x; u < g_cursor_right; u++ )
	{
       _outbyte(' ');
    }
	
    g_cursor_x = OldX;
	g_cursor_y = OldY;

}
*/


/*
 * printf_main:
 *     #todo: Esse teste de biblioteca pode pertencer � framework,
 * deve ir para o @field 3.     
 *     Essa fun��o testa a fun��o printf() e seus recursos.
 *     Obs: We can implement this test in user mode.
 * Obs:
 *     If you compile this file with
 *     gcc -Wall $(YOUR_C_OPTIONS) -DTEST_PRINTF -c printf.c
 * you will get a normal warning:
 *   printf.c:214: warning: spurious trailing `%' in format
 * this line is testing an invalid % at the end of the format string.
 *
 * this should display (on 32bit int machine) :
 *
 * Hello world!
 * printf test
 * (null) is null pointer
 * 5 = 5
 * -2147483647 = - max int
 * char a = 'a'
 * hex ff = ff
 * hex 00 = 00
 * signed -3 = unsigned 4294967293 = hex fffffffd
 * 0 message(s)
 * 0 message(s) with %
 * justif: "left      "
 * justif: "     right"
 *  3: 0003 zero padded
 *  3: 3    left justif.
 *  3:    3 right justif.
 * -3: -003 zero padded
 * -3: -3   left justif.
 * -3:   -3 right justif.
 *
 */
/*	 
int printf_main (void){
	
	int mi;
	int i = 5;
	unsigned int bs = sizeof(int)*8;

	char *np = 0;	
	char *ptr = "Hello world!";
	char buf[80];
	
	

	mi = (1 << (bs-1)) + 1;
	
//	vsync();
	
	printf("%s\n", ptr);
	printf("printf test\n");
	printf("%s is null pointer\n", np);
	printf("%d = 5\n", i);
	printf("%d = - max int\n", mi);
	printf("char %c = 'a'\n", 'a');
	printf("hex %x = ff\n", 0xff);
	printf("hex %02x = 00\n", 0);
	printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
	printf("%d %s(s)%", 0, "message");
	printf("\n");
	printf("%d %s(s) with %%\n", 0, "message");
	
    sprintf(buf, "justif: \"%-10s\"\n", "left"); 
    printf("%s", buf);
	
    sprintf(buf, "justif: \"%10s\"\n", "right"); 
    printf("%s", buf);
	
    sprintf(buf, " 3: %04d zero padded\n", 3); 
    printf("%s", buf);
	
    sprintf(buf, " 3: %-4d left justif.\n", 3); 
    printf("%s", buf);
	
    sprintf(buf, " 3: %4d right justif.\n", 3); 
    printf("%s", buf);
	
    sprintf(buf, "-3: %04d zero padded\n", -3); 
    printf("%s", buf);
	
    sprintf(buf, "-3: %-4d left justif.\n", -3); 
    printf("%s", buf);
	
    sprintf(buf, "-3: %4d right justif.\n", -3); 
    printf("%s", buf);


done:
	return (int) 0;
};
*/


/*
 ******************************************************************
 * input:
 *     Coloca os caracteres digitados em um buffer, (string). 
 * Para depois comparar a string com outra string, que � um comando.
 * 
 *     Devemos nos certificar que input(.) n�o imprima nada.
 *
 * History:
 *     2015 - Created by Fred Nora.
 *     ...
 */
unsigned long input ( unsigned long ch ){
	
	int i;
	
	// Converte 'unsigned long' em 'char'.
	char c = (char) ch;  	

    if( g_inputmode == INPUT_MODE_LINE )	
    {
        //Limite.
	    if(prompt_pos >= PROMPT_SIZE)
	    {
	        printf("input: INPUT_MODE_LINE full buffer!\n");	
	        refresh_screen();
			return (unsigned long) 0; 
	    };
    };
	
	if(g_inputmode == INPUT_MODE_MULTIPLE_LINES )
	{
		//tem que ter o tamanho de um arquivo.
		if(prompt_pos >= PROMPT_SIZE)
		{
	        printf("input: INPUT_MODE_MULTIPLE_LINES full buffer!\n");	
	        refresh_screen();
			return (unsigned long) 0; 			
		}
	};
	
	
	// Trata o caractere digitado. 
	
	switch (c)
	{
		
	    //Enter.	
		//+se for modo comando devemos finalizar com zero.
		//+se for modo texto, devemos apenas incluir os caracteres \r \n.		
		//case 0x1C:
		case VK_RETURN:
            //modo linha 
			if(g_inputmode == INPUT_MODE_LINE )
			{
			    prompt[prompt_pos] = (char )'\0'; //end of line.
			    //@todo: ?? ldiscCompare();
				//o compare est� no aplicativo.
	            for(i=0; i<PROMPT_MAX_DEFAULT;i++)
	            {
		            prompt[i] = (char) '\0';
		            prompt_out[i] = (char) '\0';
		            prompt_err[i] = (char) '\0';
	            };
                prompt_pos = 0;				
				goto input_done;
			};
            //modo multiplas linhas 
		    if(g_inputmode == INPUT_MODE_MULTIPLE_LINES ){
			    prompt[prompt_pos] = (char )'\r';
                prompt_pos++;
				prompt[prompt_pos] = (char )'\n';
				prompt_pos++;				
			};			
		    break;

	    //Backspace.
		case 0x0E:
		
            if ( prompt_pos <= 0 ){
				
			    prompt_pos = 0;
				prompt[prompt_pos] = (char ) '\0';
				break; 
			}
		    
			//Apaga o anterior (no buffer).
			prompt_pos--;
			prompt[prompt_pos] = (char ) '\0';
			break;
			
		//...	
		
        //Para qualquer caractere que n�o sejam os especiais tratados acima.		
		default:
		    prompt[prompt_pos] = c;
		    prompt_pos++;          //incrementa fila
			break;
	};
	
input_more:	
	return 0;
input_done:	
    return VK_RETURN;	
};



/*
 *****************
 * stdioInitialize:
 *     Inicializando stdio pertencente ao kernel base.
 *     Inicializa as estruturas do fluxo padr�o.
 *     Quem chamou essa inicializa��o ?? Em que hora ??
 */
int stdioInitialize (){
	
	int Status = 0;
	
	int i;

	// Buffers para as estruturas.
	unsigned char *buffer0;
	unsigned char *buffer1;
	unsigned char *buffer2;
	
	int cWidth = get_char_width ();
	int cHeight = get_char_height ();
	
	if ( cWidth == 0 || cHeight == 0 )
	{
		//#debug
		printf("stdioInitialize: fail w h ");
		die();
	}	
	

	// #bugbug:
	//  4KB alocados para cada estrutura. Isso � muito.
	//  Mas ao mesmo tempo estamos economizando o heap 
	//  usado pelo malloc.
	//  Podemos alocar 4KB para o buffer. 'prompt'
	
	
	// Alocando uma p�gina para cada buffer.
	// 4KB size.
	// #importante
	// Obs: Essas p�ginas s�o alocadas em user mode.
	
	
	//4KB
	buffer0 = (unsigned char *) newPage(); 
	if( (unsigned char *) buffer0 == NULL )
	{
		printf("buffer0\n");
        goto fail;
	}
	
	//4KB
	buffer1 = (unsigned char *) newPage();
	if( (unsigned char *) buffer1 == NULL )
	{
		printf("buffer1\n");
        goto fail;
	}
	
	//4KB
	buffer2 = (unsigned char *) newPage();
	if( (unsigned char *) buffer2 == NULL )
	{
		printf("buffer2\n");
        goto fail;
	}
	
	//
	// Alocando mem�ria para o fluxo padr�o do 
	// processo kernel.
	// Estamos apenas alocando mem�ria para a estrutura.
	//
	
	stdin = (FILE *) &buffer0[0];	
	stdout = (FILE *) &buffer1[0];	
	stderr = (FILE *) &buffer2[0];	
	  

    // Configurando a estrutura de stdin. 
	stdin->_base = &prompt[0];
	stdin->_ptr =  &prompt[0];
	stdin->_cnt = PROMPT_MAX_DEFAULT;
	stdin->_file = 0;
	stdin->_tmpfname = "k-stdin";
	//...

    // Configurando a estrutura de stdout.
	stdout->_base = &prompt_out[0];
	stdout->_ptr = &prompt_out[0];
	stdout->_cnt = PROMPT_MAX_DEFAULT;
	stdout->_file = 1;
	stdout->_tmpfname = "k-stdout";
	//...
	
    // Configurando a estrutura de stderr.
	stderr->_base = &prompt_err[0];
	stderr->_ptr =  &prompt_err[0];
	stderr->_cnt = PROMPT_MAX_DEFAULT;
	stderr->_file = 2;
	stderr->_tmpfname = "k-stderr";	
	//...
	
	
	//
    // #importante
    // Salvando os ponteiros na lista de arquivos.	
	//
	Streams[0] = (unsigned long) stdin;
	Streams[1] = (unsigned long) stdout;
	Streams[2] = (unsigned long) stderr;
	
	//Os pr�ximos s�o inicializados em fs.c
	//Streams[3] volume0 root dir (vfs) 
	//Streams[4] volume1 root dir (boot volume)
    //Streams[5] volume2 root dir  (system volume)	 
	//...  	
	
	
	
    //Configurando o array global para ser o 
    //mesmo que o array local.	
	gStreams = (unsigned long *) &Streams[0];
	
	//N�mero de streams no array global
	g_nstream = NUMBER_OF_FILES;
	
	//
	// Flag para o tipo de input.
	// # Multiplas linhas.
	//
	
	g_inputmode = INPUT_MODE_MULTIPLE_LINES;
	
	
	//  ## Cursor ##
	
	// Inicializa o cursor com margens bem abertas.	
	
	g_cursor_left = (0);
	g_cursor_top = (0); 

	//@todo:
	//Isso � complicado.
	//Temos que pegar esses valores.
	//g_cursor_width = ?;
	//g_cursor_height = ?;	
	
	//#bugbug 
	//#todo: precisamos as dimens�es do char com base na fonte antes de tudo.
	//?? Talvez aqui devamos inicializar a fonte e as dimens�es de char..
	//pois estamos na inicializa��o da libc. Mas isso ja pode ter acontecido na 
	//inicializa��o do kernel, quando era necess�rio algum tipo de fonte para 
	//mensagens de debug.
	//?? ser� que j� configuramos as dimens�es da tela. Devemos usar as vari�veis.
	//?? se as dimens�es da tela foram configuradas na inicializa��o,
	//ent�o podemos usar as vari�veis aqui.
	
	//precisamos saber as dimens�es da tela e do char.
	//g_cursor_right = (800/8);
	//g_cursor_bottom = (600/8);
	g_cursor_right = (800/cWidth);
	g_cursor_bottom = (600/cHeight);
	
	
	
    //x e y.
	g_cursor_x = g_cursor_left; 
	g_cursor_y = g_cursor_top;  		
	
	// Default color.
	// N�o sabemos se o esquema de cores do sistema j�
	// est� configurado, ent�o usaremos uma cor padr�o.
	// A QUALQUER HORA O KERNEL PODE ESCREVER NO TERMINAL 
	// E PARA USARMOS JANELAS PRETAS TEMOS QUE CONFIGURA A 
	// COR DA FONTE, ENT�O JANELAS TER�O FONTE PRETA.
	g_cursor_color = COLOR_TERMINALTEXT;
	
	
	//
	// #importante:
	// Preenche os arquivos do fluxo padr�o do kernel base
	// com 'zeros'.
	//

	for ( i=0; i<PROMPT_MAX_DEFAULT; i++ ){
		
		prompt[i] = (char) '\0';
		prompt_out[i] = (char) '\0';
		prompt_err[i] = (char) '\0';
	};

	// Inicializa o deslocamento dentro do arquivo de entrada.
	// #bugbug @todo: Poder�amos ter posicionamento dentro 
	// dos 3 arquivos. Mas isso reflete apenas o posicionamento 
	// dentro de stdin por enquanto.
	
    prompt_pos = 0;	
	
	// Done !
	
//done:

    return (int) 0;	
	
fail:
    panic ("uitm-libc-stdio-stdioInitialize: fail\n");
};


//
// End.
//
