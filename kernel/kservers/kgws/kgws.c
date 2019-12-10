/*
 * File: kgws.c
 * 
 *     kgws - kernel gramado window server.
 * 
 * History:
 *     2019 -  Created by fred Nora.
 */
 
// Window Server into the kernel base.

#include <kernel.h>

int kgws_status;
int kgws_ws_PID;
int kgws_ws_status;


int kgws_mouse_event_mouse_buttom_1;
int kgws_mouse_event_mouse_buttom_2;
int kgws_mouse_event_mouse_buttom_3;

int kgws_mouse_event_old_mouse_buttom_1;
int kgws_mouse_event_old_mouse_buttom_2;
int kgws_mouse_event_old_mouse_buttom_3;


unsigned long kgws_mouse_event_saved_mouse_x;
unsigned long kgws_mouse_event_saved_mouse_y;
unsigned long kgws_mouse_event_mouse_x;
unsigned long kgws_mouse_event_mouse_y;

int kgws_mouse_event_moving;
int kgws_mouse_event_drag_status;

int kgws_mouse_event_button_pressed;

//se houve ou não alguma ação envolvendo botões.
int kgws_mouse_event_button_action;


int flagRefreshMouseOver;


//salvaremos aqui o último total ticks pra
//pegarmos um delta, se o delta for menor que o limite
//então temos um duploclick.
unsigned long kgws_current_totalticks;
unsigned long kgws_last_totalticks;
unsigned long kgws_delta_totalticks;


// #bugbug
// Algumas variáveis usadas aqui estão no driver de mouse ps2
// Precisamos delas aqui, pois não possuem utilidade lá.
// aqui o servidor de janelas escaneia as janelas para saber 
// se o mouse está sobre alguma ... durante a rotina
// são solicitadas informações diretamente no driver de mouse ps2.

int kgws_mouse_scan_windows (void)
{
	
	// #importante:
	// Essa será a thread que receberá a mensagem.
	struct thread_d *t;


	// #importante:
	// Essa será a janela afetada por qualquer evento de mouse.
	// ID de janela.

    struct window_d *Window;
	int wID;

    //#test
    int last_wID; 
    
    
    //
    // Chamar o driver de mouse ps2 pra pegar as informações
    // sobre o mouse;
    //
    
    // #todo
    // Temos que pegar um pacote com todas as informações de uma vez.

    //ok
	kgws_mouse_event_saved_mouse_x = ps2_mouse_get_info (1);
	kgws_mouse_event_saved_mouse_y = ps2_mouse_get_info (2);
	kgws_mouse_event_mouse_x = ps2_mouse_get_info (3);
	kgws_mouse_event_mouse_y = ps2_mouse_get_info (4);


    kgws_mouse_event_moving = ps2_mouse_get_info (5);
    
    kgws_mouse_event_drag_status = ps2_mouse_get_info (6);
    
    kgws_mouse_event_button_action = ps2_mouse_get_info (7);


	kgws_mouse_event_mouse_buttom_1 = ps2_mouse_get_info (8);
	kgws_mouse_event_mouse_buttom_2 = ps2_mouse_get_info (9);
	kgws_mouse_event_mouse_buttom_3 =  ps2_mouse_get_info (10);

	kgws_mouse_event_old_mouse_buttom_1 = ps2_mouse_get_info (11);
	kgws_mouse_event_old_mouse_buttom_2 =  ps2_mouse_get_info (12);
	kgws_mouse_event_old_mouse_buttom_3 =  ps2_mouse_get_info (13);
    
    kgws_mouse_event_button_pressed =  ps2_mouse_get_info (14);
    

    //printf ("b=%d ",kgws_mouse_event_mouse_buttom_1);

	// #refletindo: 
	// ?? E no caso de apenas considerarmos que o mouse está se movendo, 
	// mandaremos para janela over. ???
	
	// #refletindo:
	// Obs: A mensagem over pode ser enviada apenas uma vez. 
	// será usada para 'capturar' o mouse ... 
	// e depois tem a mensagem para 'descapturar'.
	
	
    // ## Sem escaneamento de janelas por enquanto, apenas mostre e mova o ponteiro ##
	
	//## então não enviaremos mensagens para a thread ###
	
	//
	//  ## Scan ##
	//
	
	//===========
	// (capture) - On mouse over. 
	//

	// wID = ID da janela.
	// Escaneamos para achar qual janela bate com os valores indicados.
	// Ou seja. Sobre qual janela o mouse está passando.
	
	// #BUGBUG
	// O problema nessa técnica são as sobreposição de janelas.
	// Quando uma janela está dentro da outr, então duas janelas
	// estão áptas a serem selecionadas.
	// Talvez devamos filtrar e só aceitarmos sondar procurando 
	// por controles.

	// #IMPORTANTE
	// Se for válido e diferente da atual, significa que 
	// estamos dentro de uma janela.
	// -1 significa que ouve algum problema no escaneamento.
	
	//wID = (int) windowScan ( mouse_x, mouse_y );

    
	//#importante:
	//Ja que não passamos em cima de um botão ou editbox,
	//então vamos ver se estamos em cima de uma janela overlapped.
    //#obs: isso ficou bom ... estamos testando
    
    // #bugbug
    // Com isso o sistema trava quando tentamos mover o mouse
    // no programa gdeshell, que é fullscreen.
    //Podemos tentar desabilitar o mouse quando entrarmos em fullscreen
    // no gdeshell(teste)?
    
    //#todo: 
    //Não há porque sondar janelas se tivermos em full screen.
    //pois somente teremos a área de cliente de uma das janelas.

    //============
    //if ( wID == -1 )
    //{
         //wID = (int) windowOverLappedScan ( mouse_x, mouse_y );
   // }


    //wID = (int) windowOverLappedScan ( mouse_x, mouse_y );
    
    //se pegamos uma overlapped.
    //vamos testar os botoes
    //if ( wID != -1 )
    //{
        //printf ("o ");
		//wID = (int) windowScan ( mouse_x, mouse_y );
    //}
    
    
    wID = (int) windowScan ( kgws_mouse_event_mouse_x, kgws_mouse_event_mouse_y );


    //================
	//Se houve problema no escaneamento de janela do tipo botão ou editbox.
    if ( wID == -1 )
    { 

		// Essa flag indica que podemos fazer o refresh da mouse ouver,
		// mas somente uma vez.
        if ( flagRefreshMouseOver == 1 )
        {
            Window = (struct window_d *) windowList[mouseover_window];

			//#bugbug:
			//precisamos checar a validade da estrutura antes de usa-la.

			//#importante:
			//isso apaga o que pintamos na janela, o valor foi salvo logo abaixo.

            if ( (void *) Window != NULL )
            {
                //refresh_rectangle ( Window->left, Window->top, 20, 20 );
                //refresh_rectangle ( savedmouseoverwindowX, savedmouseoverwindowY, 20, 20 );

            }

			// Não podemos mais fazer refresh.
            flagRefreshMouseOver = 0;

			//#importante inicializa.
			//isso é global do sistema?
            mouseover_window = 0;
        }
     }
     
     
    if ( wID == -1 )
    {
		return -1;
    }


    //++
	// Se não houve problema no escaneamento de janela ou seja, se encontramos 
	// uma janela. Então essa janela deve estar associada à uma thread para qual 
	// mandaremos a mensagem. Caso a thread for null ... apenas não enviamos.
	// A janela tem uma thread de controle, igual ao processo.

     //============================ 
     //Se estamos sobre uma janela válida.
     if ( wID > -1 )
     {

        Window = (struct window_d *) windowList[wID];

        if ( (void *) Window == NULL ) { return; }
        if ( Window->used != 1 || Window->magic != 1234){ return; }


		//#importante:
		//Nesse momento temos uma janela válida, então devemos 
		//pegar a thread associada à essa janela, dessa forma 
		//enviaremos a mensagem para a thread do aplicativo ao qual 
		//a janela pertence.

        t = (void *) Window->control;

        if ( (void *) t == NULL ){ return; }
        if ( t->used != 1 || t->magic != 1234 ){ return; }

		//#bugbug 
		//#todo:
		// Deveriamos aqui checarmos a validade da estrutura ??

		//redraw_window(wScan);


        //
        // ==== Button events ====
        //
 
		// #importante
		// Se um botão foi pressionado ou liberado, então enviaremos uma 
		// mensagem relativa ao estado do botão, caso contrário, enviaremos 
		// uma mensagem sobre a movimentação do mouse.

		//Qual botão mudou seu estado??
		//Checaremos um por um.


		//===============================================
		// ***Se houve mudança em relação ao estado anterior.
		// Nesse momento um drag pode terminar
        if ( kgws_mouse_event_button_action == 1 )
        {
			//printf ("[Action ");

			// >> BOTÃO 1 ==================
			//Igual ao estado anterior
            if ( kgws_mouse_event_mouse_buttom_1 == kgws_mouse_event_old_mouse_buttom_1 )
            {
				//...
			
			//Diferente do estado anterior.
            }else{
				
				//printf ("Dif ");
				
				// down - O botão 1 foi pressionado.
                if ( kgws_mouse_event_mouse_buttom_1 == 1 )
                {
					//printf ("Press\n");
					 
					//clicou
					// se o old estava em 0 então o atual está em um.
                    if ( kgws_mouse_event_old_mouse_buttom_1 == 0 )
                    {
						// flag: um botão foi pressionado.
						kgws_mouse_event_button_pressed = 1;
						
						// Enviaremos a mensagem para a thread atual.
						// houve alteração no estado do botão 1 e estamos em cima de uma janela.
                        if ( (void *) Window != NULL )
                        {
							//pegamos o total tick
							kgws_current_totalticks = (unsigned long) get_systime_totalticks();
                            kgws_delta_totalticks = (kgws_current_totalticks - kgws_last_totalticks); 
                            //printf ( "x=%d l=%d d=%d \n",
                               //kgws_current_totalticks, kgws_last_totalticks, kgws_delta_totalticks ); 
                               //refresh_screen();
                            kgws_last_totalticks = kgws_current_totalticks;
                            t->window = Window;
                            t->msg = MSG_MOUSEKEYDOWN;
                            if (kgws_delta_totalticks < 1000) //2000
                            {
								t->msg = MSG_MOUSE_DOUBLECLICKED; 
								kgws_delta_totalticks=8000; // delta inválido.
							}
                            t->long1 = 1;
                            t->long2 = 0;
                            t->newmessageFlag = 1;   
                        }
                        //else: // houve alteração no estado do botão 1 mas não estamos em cima de uma janela.
                        
                        
						//Atualiza o estado anterior.
                        //kgws_mouse_event_old_mouse_buttom_1 = 1;
                        kgws_mouse_event_old_mouse_buttom_1 = kgws_mouse_event_mouse_buttom_1;
                    }

				// up - O botão 1 foi liberado.
                }else{
					
					//printf ("Rel\n");

				    // flag: um botão foi liberado.
				    kgws_mouse_event_button_pressed = 0;
						
					// #importante 
					// Enviaremos a mensagem para a thread atual.
                    if ( (void *) Window != NULL )
                    {
						// Não estamos mais carregando um objeto.
						//kgws_mouse_event_drag_status = 0;
                        
                        t->window = Window;
                        t->msg = MSG_MOUSEKEYUP;
                        t->long1 = 1;
                        t->long2 = 0;
                        t->newmessageFlag = 1;
                    }

                    //kgws_mouse_event_old_mouse_buttom_1 = 0;
                    kgws_mouse_event_old_mouse_buttom_1 = kgws_mouse_event_mouse_buttom_1;
                };
            }; 

            /*
			// >> BOTÃO 2 ==================
			// Igual ao estado anterior
            if ( kgws_mouse_event_mouse_buttom_2 == kgws_mouse_event_old_mouse_buttom_2 )
            {
				//...

			// Diferente do estado anterior.
            }else{

				// down - O botão 2 foi pressionado.
                if ( kgws_mouse_event_mouse_buttom_2 == 1 )
                {

					//clicou
                    if ( kgws_mouse_event_old_mouse_buttom_2 == 0 )
                    {
						// flag: um botão foi pressionado.
						kgws_mouse_event_button_pressed = 1;
						
						// houve alteração no estado do botão 2 e estamos em cima de uma janela.
                        if ( (void *) Window != NULL )
                        {
                            t->window = Window;
                            t->msg = MSG_MOUSEKEYDOWN;
                            t->long1 = 2;
                            t->long2 = 0;
                            t->newmessageFlag = 1;
                            
                        }
                        //else: // houve alteração no estado do botão 2 mas não estamos em cima de uma janela.

						// atualiza o estado anterior.
                        //kgws_mouse_event_old_mouse_buttom_2 = 1;
                        kgws_mouse_event_old_mouse_buttom_2 = kgws_mouse_event_mouse_buttom_2;
                    }

				// up - O botão 2 foi liberado.
                }else{

				    // flag: um botão foi liberado.
				    kgws_mouse_event_button_pressed = 0;
				    
                   if ( (void *) Window != NULL )
                   {
						// Não estamos mais carregando um objeto.
						//kgws_mouse_event_drag_status = 0;

                        t->window = Window;
                        t->msg = MSG_MOUSEKEYUP;
                        t->long1 = 2;
                        t->long2 = 0;
                        t->newmessageFlag = 1;
                        
                    }

                    //kgws_mouse_event_old_mouse_buttom_2 = 0;
                    kgws_mouse_event_old_mouse_buttom_2 = kgws_mouse_event_mouse_buttom_2;
                }
            }; 
            */

            /* 
			// >> BOTÃO 3 ==================
			// Igual ao estado anterior
            if ( kgws_mouse_event_mouse_buttom_3 == kgws_mouse_event_old_mouse_buttom_3 )
            {
				//...

			// Diferente do estado anterior.
            }else{

				// down - O botão 3 foi pressionado.
                if ( kgws_mouse_event_mouse_buttom_3 == 1 )
                {
					//clicou
                    if ( kgws_mouse_event_old_mouse_buttom_3 == 0 )
                    {
						// flag: um botão foi pressionado.
						kgws_mouse_event_button_pressed = 1;
						
                        // houve alteração no estado do botão 3 e estamos em cima de uma janela.
                        if ( (void *) Window != NULL )
                        {
                            t->window = Window;
                            t->msg = MSG_MOUSEKEYDOWN;
                            t->long1 = 3;
                            t->long2 = 0;
                            t->newmessageFlag = 1;
                            
                        }
                        //else: // houve alteração no estado do botão 1 mas não estamos em cima de uma janela.

						// Atualiza o estado anterior.
                        //kgws_mouse_event_old_mouse_buttom_3 = 1;
                        kgws_mouse_event_old_mouse_buttom_3 = kgws_mouse_event_mouse_buttom_3;
                    }

				// up - O botão 3 foi liberado.
                }else{

				    // flag: um botão foi liberado.
				    kgws_mouse_event_button_pressed = 0;
				    
                    if ( (void *) Window != NULL )
                    {
						// Não estamos mais carregando um objeto.
						//kgws_mouse_event_drag_status = 0;

                        t->window = Window;
                        t->msg = MSG_MOUSEKEYUP;
                        t->long1 = 3;
                        t->long2 = 0;
                        t->newmessageFlag = 1;
                        
                    }

                    //kgws_mouse_event_old_mouse_buttom_3 = 0;
                    kgws_mouse_event_old_mouse_buttom_3 = kgws_mouse_event_mouse_buttom_3;
                }
            }; 
            */ 

			// Ação concluída.
            kgws_mouse_event_button_action = 0;
            return 0;
        };


        /*
        //===============================================
        // *** Se NÃO ouve alteração no estado dos botões, então apenas 
        // enviaremos a mensagem de movimento do mouse e sinalizamos 
        // qual é a janela que o mouse está em cima.
        // Não houve alteração no estado dos botões, mas o mouse
        // pode estar se movendo com o botão pressionado.
        //a não ser que quando pressionamos o botão ele envie várias
        //interrupções, igual no teclado.

        if ( kgws_mouse_event_button_action == 0 )
        {
			//printf ("[ No Action \n");

			// #importante
			// Lembrando que estamos dentro de uma janela ...
			// por isso a mensagem é over e não move.

			//Obs: Se a janela for a mesma que capturou o mouse,
			//então não precisamos reenviar a mensagem.
			//Mas se o mouse estiver em cima de uma janela diferente da 
			//que ele estava anteriormente, então precisamos enviar uma 
			//mensagem pra essa nova janela.

			//#bugbug:
			//estamos acessando a estrutura, mas precisamos antes saber se ela é válida.

            //estamos em cima de uma janela e não houve alteração no estado dos botões
            if ( (void *) Window != NULL )
            {
                // diferente de mouseover window.
                if ( Window->id != mouseover_window )
                {

                    if ( mouseover_window != 0 )
                    {

                       //if ( (void *) Window != NULL ){
                        t->window = (struct window_d *) windowList[mouseover_window];
                        t->msg = MSG_MOUSEEXITED;
                        t->long1 = 0;
                        t->long2 = 0;
                        t->newmessageFlag = 1;
                        //return 0;
                        //}
                    };


					// Já que entramos em uma nova janela, vamos mostra isso.

					//botão.
					//#provisório ...
					//Isso é um sinalizador quando mouse passa por cima.
					//#test: Vamos tentar modificar as características do botão.

                    //#ok isso funciona, vamos deixar oapp fazer isso.
                    //faremos isso somente para os botões do sistema.  
                    //if ( Window->isButton == 1 )
                    //{
						//isso funciona.
                        //bmpDisplayCursorBMP ( fileIconBuffer, Window->left, Window->top );
						//#test
                        //update_button ( (struct button_d *) Window->button,
                            //(unsigned char *) Window->button->string,
                            //(int) Window->button->style,
                            //(int) BS_HOVER,
                            //(int) Window->button->type,
                            //(unsigned long) Window->button->x, 
                            //(unsigned long) Window->button->y, 
                            //(unsigned long) Window->button->width, 
                            //(unsigned long) Window->button->height, 
                            //(unsigned long) Window->button->color );
						//redraw_button ( (struct button_d *) Window->button );
						//show_window_rect (Window);
					//};
					

				    //não botão.
				    //if ( Window->isButton == 0 )
				    //{
				    //    bmpDisplayCursorBMP ( folderIconBuffer, Window->left, Window->top );
				    //};


				    //nova mouse over
                    mouseover_window = Window->id;

					//#importante:
					//flag que ativa o refresh do mouseover somente uma vez.
                    flagRefreshMouseOver = 1;
                    

					// Agora enviamos uma mensagem pra a nova janela que 
					// o mouse está passando por cima.
                    //#todo: reagir a isso lá nos apps.
                    
                    t->window = Window;
                    t->msg = MSG_MOUSEOVER;
                    t->long1 = 0;
                    t->long2 = 0;
                    t->newmessageFlag = 1;
                    
                    //return 0;

                //É mouse over window.
                // não estamos em cima de uma janela e não houve alteração no estado dos botões
                }else{ 

				    //nothing ...
                };
            };

			// Ação concluída.
			// Para o caso de um valor incostante na flag.
            kgws_mouse_event_button_action = 0;
            return 0;
        };
        */




    };
    //--
}






// Registrar um window server.
int kgwsRegisterWindowServer ( int pid ){

    int Status = 0;


    if ( kgws_status != 1 )
    {
		Status = 1;
		goto fail;
    }else{
		
	    kgws_ws_PID = (int) pid;	
	    kgws_ws_status = 1;
		goto done;
    };


fail:
    printf("kgwsRegisterWindowServer: fail\n");

done:
    return 0;
}





