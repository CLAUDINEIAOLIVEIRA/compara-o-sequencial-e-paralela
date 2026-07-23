#include <stdio.h> /*biblioteca responsável pelos comandos de entrada e saída ("printf", "scanf", "fopen" e "fprintf")*/
#include <time.h> /*biblioteca responsável pelos comandos de medição de tempo ("clock" e "CLOCK_PER_SEC")*/
#include <stdlib.h> /*biblioteca responsável pelos comandos que geram números aleatórios ("rand()" e "srand()")*/
#include <thread> /*biblioteca responsável por gerenciar as Threads nativas da linguagem C++*/
#include <omp.h> /*biblioteca responsável pelo OpenMP que fornece as diretivas que fazem o compilador paralelizar os laços de repetição automaticamente*/

FILE *arquivo; /*uma variável global (ponteiro para um arquivo) que permite que qualquer parte do código possa adicionar informações no arquivo*/

/*função responsável por executar o algoritmo de ordenação Insertion Sort, a qual foi disponibilizada no site do professor*/
void insertionsort(int *b, int tamanho_b)
{
    for (int i = 1; i < tamanho_b; i++)
    {
    	int e = b[i];
        int j = i - 1;
        while (j >= 0 && b[j] > e)
        {
            b[j + 1] = b[j];
            j--;
        }
        b[j + 1] = e;
    }
}

/*função responsável por executar o algoritmo de ordenação Selection Sort, a qual foi disponibilizada no site do professor*/
void selectionsort(int *c, int tamanho_c)
{
    for (int i = 0; i < tamanho_c - 1; i++)
    {
        int min = i;
        for (int j = i + 1; j < tamanho_c; j++)
        {
            if (c[j] < c[min])
            {
                min = j;
            }
        }
        if (min != i)
        {
            int aux = c[i];
            c[i] = c[min];
            c[min] = aux;
        }
    }
}

/*função responsável por executar a primeira parte do algoritmo de ordenação Merge Sort, a qual foi disponibilizada no site do professor*/
void mergesort_1(int *d, int ini, int fim, int meio)
{
    int pos1 = ini;
    int pos2 = meio + 1;
    int posc = 0;
    int *c = new int[fim - ini + 1];
    
    while (pos1 <= meio && pos2 <= fim)
    {
        if (d[pos1] < d[pos2])
        {
            c[posc] = d[pos1];
            pos1++;
        } 
        else
        {
            c[posc] = d[pos2];
            pos2++;
        }
        posc++;
    }
    for (; pos1 <= meio; pos1++, posc++)
    {
        c[posc] = d[pos1];
    }
    for (; pos2 <= fim; pos2++, posc++)
    {
        c[posc] = d[pos2];
    }
    int j = 0;
    for (int i = ini; i <= fim; i++, j++)
    {
        d[i] = c[j];
    }
    delete[] c;
}

/*função responsável por executar a segunda parte do algoritmo de ordenação Merge Sort, a qual foi disponibilizada no site do professor*/
void mergesort_2(int *d, int i, int f)
{
    if (i < f)
    {
        int meio = (f + i) / 2;
        mergesort_2(d, i, meio);
        mergesort_2(d, meio + 1, f);
        mergesort_1(d, i, f, meio);
    }
}

/*função responsável por coordenadar a ordenação em um único bloco*/
void ordenarBloco(int *bloco, int tamanho)
{
    /*desvio condicional que direciona os blocos pares para o algoritmo Insertion Sort e os blocos ímpares para o algoritmo Selection Sort*/
    if (tamanho % 2 == 0) 
    {
        insertionsort(bloco, tamanho); /**/
    } 
    else
    {
        selectionsort(bloco, tamanho);
    }
}

/*função responsável por executar o processo de ordenação de forma sequencial, ou seja, recrutando apenas um único núcleo do computador. Trata-se da computação tradicional*/
void ordenarBlocosSequencial(int **blocos, int *tamanhos, int qtd_blocos) 
{
    for (int i = 0; i < qtd_blocos; i++)
    {
        ordenarBloco(blocos[i], tamanhos[i]);
    }
}

/*função responsável por executar a computação paralela via OpenMP*/
void ordenarBlocosOpenMP(int **blocos, int *tamanhos, int qtd_blocos)
{
    /*laço de repetição paralelizado, ou seja, recrutando todos os núcleos do computador para sua execução (semelhante ao "ParalellDo" do Mathematica)*/
    #pragma omp parallel for
    for (int i = 0; i < qtd_blocos; i++)
    {
        ordenarBloco(blocos[i], tamanhos[i]);
    }
}

/*estrutura que sintetiza os principais parâmetros que serão usados nas Threads.*/
struct DadosThread
{
    int **blocos; /*matriz contendo todos os blocos do programa*/
    int *tamanhos; /*lista contendo os tamanhos de cada bloco*/
    int inicio; /*indica onde a thead deve começar a trabalhar (por exemplo, o bloco 0)*/
    int fim; /*indica onde a thread deve parar (por exemplo, o bloco 1000)*/
};

/*função que cada thread irá executar*/
void rotinaThread(DadosThread dados)
{
    for (int i = dados.inicio; i < dados.fim; i++)
    {
        ordenarBloco(dados.blocos[i], dados.tamanhos[i]);
    }
}

/*função responsável pela divisão manual das tarefas para cada thread*/
void ordenarBlocosThreads(int **blocos, int *tamanhos, int qtd_blocos)
{
    int num_threads = std::thread::hardware_concurrency(); /*variável que recebe a quantidade de núcleos do computador*/
   
    /*laço de repetição que nos auxilia caso o sistema operacional falhe em indicar o número de núcleos do computador. Por uma questão de segurança, caso haja a falha na obtenção do número de núcleos do computador, assumimos que ele tem 4 núcleos*/   
    if (num_threads == 0) 
    {
    	num_threads = 4;
    }

    std::thread *threads = new std::thread[num_threads]; /*criamos uma lista de objetos do tipo std::thread, a qual é baseada no número de núcleos do computador*/
    
    /*perceba o cálculo engenhoso realizado abaixo, em que a variável "partes" recebe o número de blocos que cada thread deverá ordenar. Intuitivamente, o cálculo deveria ser qtd_blocos/num_threads, porém, este cálculo falha se a divisão não for exata. Por exemplo, se há 4002 blocos para 4 threads, então, cada uma ficaria responsável por 1000.5 blocos, porém, a variável inteira fornece como resultado apenas números inteiros (neste caso, 1000 blocos por thread. omitindo parte do vetor a ser ordenado). Com o cálculo abaixo, sempre ocorrerá o arredondamento para cima, garantindo que nenhum bloco fique de fora*/
    
    int partes = (qtd_blocos + num_threads - 1) / num_threads; /*variável que recebe o número de blocos que cada thread precisará ordenar*/


    /*laço de repetição que efetivamente passa as tarefas para cada thread*/
    for (int t = 0; t < num_threads; t++)
    {    
        DadosThread dados; /*em "dados", cada thread receberá as informações necessárias ao seu funcionamento*/
        dados.blocos = blocos; /*recebe o endereço da matriz "blocos"*/
        dados.tamanhos = tamanhos; /*recebe a lista "tamanhos" para que a thread saiba onde os dados estão na memória*/
        dados.inicio = t * partes; /*delimita o ínicio da atuação da thread*/
        dados.fim = dados.inicio + partes; /*delimita o final da atuação da thread*/
        
        /*desvio condicional que corrige o arredondamento para cima que foi feito para se obter a variável inteira "partes". O arredondamento para cima pode ultrapssar a quantidade de blocos existentes, então, se isto for verdadeiro, o desvio condicional atua, impondo o fim do bloco no último elemento do vetor a ser ordenado*/
        if (dados.fim > qtd_blocos)
        { 
            dados.fim = qtd_blocos;
        }
	/*o desvio condicional abaixo permite a criação de threads se houver dados a serem ordenados*/
        if (dados.inicio < qtd_blocos)
        {
            threads[t] = std::thread(rotinaThread, dados); /*criação das threads*/
        }
    }

    /*seção importante, pois evita que haja travamento no programa*/
    for (int t = 0; t < num_threads; t++)
    {
    	/*o desvio condicional verifica se a threat existe e está ativa*/
        if (threads[t].joinable())
        {
            threads[t].join(); /*linha importantíssima, pois ela força o programa principal a esperar que uma thread paralela termine sua execução antes de prosseguir*/
        }
    }
    delete[] threads; /*libera a memória alocada anteriormente*/
}

/*função responsável pela etapa de unificação dos blocos ordenados*/
void unirEOrdenarFinal(int **blocos, int *tamanhos, int qtd_blocos, int *resultado)
{

/*lembre-se que "**blocos" é o ponteiro para ponteiros que está associado a matriz dinâmica, apontando para a coleção de blocos, onde cada linha da matriz é um bloco de tamanho 4 ou 5 que foram ordenados.*/

/*lembre-se que "*tamanho" é a lista que guarda o tamanho de cada bloco. Por exemplo, "tamanho[0]" indica se o bloco tem 4 ou 5 elementos.*/

/*lembre-se que "qtd_blocos" é uma variável inteira que informa quantos blocos foram criados no total*/

/*lembre-se que "*resultado" é o ponteiro da lista na qual os blocos com elementos ordenados serão juntados*/

    int pos_atual = 0; /*variável auxiliar*/
    
    /*laço de repetição externo acessa cada linha da matriz dinâmica "blocos" (blocos ordenados)*/
    for (int i = 0; i < qtd_blocos; i++) 
    {
        /*laço de repetição interno acessa as colunas da matriz dinâmica "blocos" (acessa o interior de cada bloco)*/
        for (int j = 0; j < tamanhos[i]; j++)
        {
            resultado[pos_atual] = blocos[i][j]; /*dentro de um determinado bloco, copia-se um número para o vetor "resultado"*/
            pos_atual++; /*após copiar o número para o vetor, atualiza-se a posição do vetor que receberá outro número da matriz dinâmica*/
        }
    }
    
    /*após a execução dos dois laços de repetição aninhados, todos os números do vetor original estão no vetor "resultados"*/
    
    /*o laço de repetição faz uma verificação se o vetor "resultados" não está vazia, pois só faz sentido ordenar um vetor que possui elementos*/
    if (pos_atual > 0)
    {
        mergesort_2(resultado, 0, pos_atual - 1); /*chama-se o algoritmo Merge Sort para a ordenação final*/
    }
}

/*função principal*/
int main()
{
    arquivo = fopen("tempo_de_ordenacao.txt", "w"); /*cria-se um arquivo de texto que conterá os resultados da ordenação*/
    if (arquivo == NULL) /*desvio condicional que emite uma mensagem de erro caso haja problema na criação/leitura do arquivo de texto*/
    {
        printf("Erro ao abrir o arquivo de texto!\n");
    }

    /*conjunto de comandos "print" que exibirão as informações relacionadas com os resultados numéricos (tempo de ordenação)*/
    fprintf(arquivo, "Tamanho\tMedia_Sequencial(s)\tMedia_OpenMP(s)\tMedia_Threads(s)\n"); /*cabeçalho no arquivo de texto*/
    printf("Processando dados e salvando em 'tempo_de_ordenacao.txt'...\n\n"); /*mensagem no terminal, indicando a execução das tarefas requeridas*/
    printf("Tamanho\tSeq (s)\t\tOpenMP (s)\tThreads (s)\n"); /*cabeçalho no terminal*/

    /*laço de repetição externo controla o tamanho do vetor original*/
    for (int tamanho = 15000; tamanho <= 20000; tamanho += 1000)
    {
    	/*tempo total das trinta execuções de trinta vetores com o mesmo tamanho a serem ordenados*/
        double tempo_seq_total = 0; /*tempo de ordenação com a computação tradicional (sequencial), ou seja, usando um único núcleo do computador*/
        double tempo_omp_total = 0; /*tempo de ordenação com a computação paralela via OpenMP*/
        double tempo_thr_total = 0; /*tempo de ordenação com a computação paralela via threads*/

	/*laço de repetição que controla as trinta repetições para cada tamanho de vetor*/
        for (int exec = 0; exec < 30; ++exec)
        {
            
            int *original = new int[tamanho]; /*alocação dinâmica de um vetor de inteiros, o qual será nosso vetor original (não-ordenado)*/
            srand(exec + 1); /*semente que será usada na geração de números aleatórios que comporão o vetor*/
            
            /*laço de repetição que preencherá o vetor original com números aleatórios no intervalo fechado [0, 100000]*/
            for (int i = 0; i < tamanho; i++)
            {
                original[i] = rand() % 100001; 
            }

            /*a seção abaixo começa com a realização do cálculo teórico, onde se calcula o número máximo de blocos. Como não sabemos de antemão quantos blocos existirão no total - pois eles podem ter tamanho 4 ou 5 - fazemos o cálculo que maximiza a quantidade de blocos, ou seja, consideramos que todos os blocos têm tamanho 4. Por fim, adicionamos uma unidade para assegurar que tenhamos um arredondamento para cima caso haja um número máximo de blocos não-inteiro*/
            
            int max_blocos = (tamanho / 4) + 1; /*cálculo do número máximo de blocos*/
            int **blocos = new int*[max_blocos]; /*criação de uma lista que armazenará endereços de memória*/
            int *tamanhos = new int[max_blocos]; 
            int qtd_blocos = 0; /*um contador que se inicia em zero que indica qual bloco estamos mexendo no momento*/

            /*Preenchendo a estrutura de blocos (Mesmo sorteio para as 3 abordagens)*/
            srand(exec + 99); 
            int indice_orig = 0; 
            
            /*laço de repetição que continua rodando enquanto não tivermos fatiado o vetor "original" inteiro de ponta a ponta*/
            while (indice_orig < tamanho)
            {
                int tam_bloco = 4 + (rand() % 2); /*sorteia o número 0 ou 1 ("rand() % 2") que, ao ser adicionado ao número 4, resulta no tamanho do bloco como sendo 4 ou 5*/
                
                /*desvio condicional que pode ser resumido no seguinte enunciado: "se o tamanho sorteado passar do fim do vetor, ajuste o tamanho do bloco para pegar apenas o que sobrou". Em outras palavras, o desvio condicional previne o erro "Segmentation Fault"*/
                if (indice_orig + tam_bloco > tamanho)
                {
                    tam_bloco = tamanho - indice_orig;
                }
                
                blocos[qtd_blocos] = new int[tam_bloco]; /*criação do bloco de 4 ou 5 elementos*/
                tamanhos[qtd_blocos] = tam_bloco; /*registro do tamanho do bloco (4 ou 5 elementos)*/

		/*laço de repetição que faz a cópia física dos números. Note que ele pega a quantidade correta de elementos a partir do "indice_orig" do vetor "original" e os despeja linha por linha, coluna por coluna ("[m]"), dentro do bloco recém-alocado*/
                for (int m = 0; m < tam_bloco; m++)
                {
                    blocos[qtd_blocos][m] = original[indice_orig + m];
                }
                qtd_blocos++; /*avançamos para a etapa evolvendo o bloco seguinte*/
                indice_orig += tam_bloco; /*atualização da posição de corte no vetor orginal. Por exemplo, se copiamos cinco números, o índice pula cinco casas para frente para sabermos de onde continuar o próximo corte no início do "while"*/
            }

            int *resultado = new int[tamanho];
            clock_t t_0, t_1;

            /*seção contendo a medição de tempo da ordenação via computação tradicional*/
            t_0 = clock(); /*início da contagem de clocks*/
            ordenarBlocosSequencial(blocos, tamanhos, qtd_blocos);
            unirEOrdenarFinal(blocos, tamanhos, qtd_blocos, resultado);
            t_1 = clock(); /*fim da contagem de clocks*/
            tempo_seq_total += (double)(t_1 - t_0) / CLOCKS_PER_SEC; /*conversão dos clocks para o tempo em segundos*/

            /*seção muito engenhosa que permite que os blocos já ordenados por meio da computação tradicional sejam preenchidos pelos blocos não-ordenados do vetor original. Assim, a próxima ordenação via computação paralela (OpenMP) não ordenará blocos já ordenados*/
            indice_orig = 0;
            for(int b=0; b<qtd_blocos; b++)
            {
                for(int m=0; m<tamanhos[b]; m++)
                {
                    blocos[b][m] = original[indice_orig + m];
                }
                indice_orig += tamanhos[b];
            }

            /*seção contendo a medição de tempo da ordenação via computação paralela (OpenMP)*/
            t_0 = clock(); /*início da contagem de clocks*/
            ordenarBlocosOpenMP(blocos, tamanhos, qtd_blocos);
            unirEOrdenarFinal(blocos, tamanhos, qtd_blocos, resultado);
            t_1 = clock(); /*fim da contagem de clocks*/
            tempo_omp_total += (double)(t_1 - t_0) / CLOCKS_PER_SEC; /*conversão dos clocks para o tempo em segundos*/

            /*seção muito engenhosa que permite que os blocos já ordenados por meio da computação paralela (OpenMP) sejam preenchidos pelos blocos não-ordenados do vetor original. Assim, a próxima ordenação via computação paralela (threads) não ordenará blocos já ordenados*/
            indice_orig = 0;
            for(int b=0; b<qtd_blocos; b++) {
                for(int m=0; m<tamanhos[b]; m++) {
                    blocos[b][m] = original[indice_orig + m];
                }
                indice_orig += tamanhos[b];
            }

            /*seção contendo a medição de tempo da ordenação via computação paralela (threads)*/
            t_0 = clock(); /*início da contagem dos clocks*/
            ordenarBlocosThreads(blocos, tamanhos, qtd_blocos);
            unirEOrdenarFinal(blocos, tamanhos, qtd_blocos, resultado);
            t_1 = clock(); /*fim da contagem dos clocks*/
            tempo_thr_total += (double)(t_1 - t_0) / CLOCKS_PER_SEC; /*conversão do número de clocks para o tempo em segundos*/

            /*laço de repetição que auxilia na liberação da memória alocada nesta rodada*/
            for (int b = 0; b < qtd_blocos; b++)
            {
                delete[] blocos[b];
            }
            
            /*liberação de memória alocada*/
            delete[] blocos;
            delete[] tamanhos;
            delete[] resultado;
            delete[] original;
        }

	/*tempo médio para cada tipo de computação testa*/
        double media_seq = tempo_seq_total / 30.0; /*tempo médio para a computação tradicional (sequencial)*/
        double media_omp = tempo_omp_total / 30.0; /*tempo médio para a computação paralela via OpenMP*/
        double media_thr = tempo_thr_total / 30.0; /*tempo médio para a computação paralela via threads*/

        /*o comando "print" exibe os resultados da medição dos tempos*/
        printf("%d\t%.6f\t\t%.6f\t\t%.6f\n", tamanho, media_seq, media_omp, media_thr);
        
        /*o comando "fprintf" escreve os resultados no arquivo de texto*/
        fprintf(arquivo, "%d\t%.10lf\t%.10lf\t%.10lf\n", tamanho, media_seq, media_omp, media_thr);
    }

    fclose(arquivo); /*fecha o arquivo*/
    printf("\nO arquivo tempo_de_ordenacao.txt foi gerado com sucesso!\n");
}












