# Estudo comparativo entre a computação sequencial e a computação paralela aplicadas aos algoritmos de ordenação

Trabalho da disciplina de Estrutura de Dados e Algoritmos — UFF/EEIMVR, Mestrado em Modelagem Computacional em Ciência e Tecnologia (MCCT).

**Autores:** Claudineia Moreira de Oliveira, Leandro Estevão de Mello, Lieger Duarte de Oliveira Rosa, Neideson Tavares Viana

## Conteúdo

- `Trabalho_de_Estruturas_de_Dados_e_Algoritmos.tex` — código-fonte LaTeX do artigo (formato ICASSP/spconf.sty)
- `Trabalho_de_Estruturas_de_Dados_e_Algoritmos.pdf` — artigo compilado
- `algoritmo.cpp` — implementação em C++ do algoritmo de ordenação híbrido (blocos ordenados por Insertion Sort/Selection Sort e unidos via Merge Sort), comparando computação sequencial, OpenMP e threads nativas

## Resumo

O trabalho compara o desempenho da computação sequencial e da computação paralela (via OpenMP e via std::thread) na ordenação de vetores de 15.000 a 20.000 elementos inteiros. Os elementos são divididos em blocos de quatro ou cinco posições, ordenados individualmente (Insertion Sort / Selection Sort) e depois unidos via Merge Sort. Os resultados mostram que a paralelização, quando mal ajustada (overhead de sincronização, ausência de num_threads), pode não superar — ou até perder para — a computação sequencial.
