# Trabalho 4 - AEDs III

## Compilação main makefile
```bash
# Entrar na pasta src
cd src

# Compilar tudo (tp4 e testador)
make

# Executar o testador
make test

# Ou executar diretamente
./testador
```

## 📊 Resumo das 25 instâncias (até 5000 itens)

Pequenas (n ≤ 200)
|Categoria |	Tipo |	n |	W% |	Correlação	|Característica especial|
|-|-|-|-|-|-|						
1|	Tamanho|	tiny_low|	50	|30%	|nenhuma	-
2|	Tamanho	|tiny_med	|50	|50%	|nenhuma	-
3|	Tamanho	|tiny_high	|50	|70%	|nenhuma	-
4|	Tamanho	|small_low	|100|	30%	|nenhuma	-
5|	Tamanho	|small_med	|100	|50%|	nenhuma	-
6|	Tamanho	|small_high	|100	|70%	|nenhuma	-
7|	Tamanho	|medium_low	|200	|30%	|nenhuma	-
8|	Tamanho	|medium_med	|200	|50%	|nenhuma	-
9|	Tamanho	|medium_high	|200	|70%	|nenhuma	-


Médias (n = 500, 1000)	
|Categoria |	Tipo |	n |	W% |	Correlação	|Característica especial|
|-|-|-|-|-|-|					
10|	Tamanho|	large_low|	500	|30%|	nenhuma	-
11|	Tamanho	|large_med	|500	|50%	|nenhuma	-
12|	Tamanho	|large_high	|500	|70%	|nenhuma	-
13|	Correlação|	uncorr_1000	|1000	|50%	|nenhuma	-
14|	Correlação|	poscorr_1000|	1000	|50%|	positiva forte	v = w + U[1,50]
15|	Correlação|	negcorr_1000|	1000	|50%	|negativa forte	v = 200 - w + U[1,30]

Grandes (n = 2000, 3000)	
|Categoria |	Tipo |	n |	W% |	Correlação	|Característica especial|
|-|-|-|-|-|-|					
16|	Estrutura|	uniform_2000|	2000|	50%|	nenhuma	uniforme [1,100]
17|	Estrutura|	clustered_2000|	2000	|50%|	nenhuma	10 clusters
18|	Estrutura|	outlier_2000|	2000	|50%|	nenhuma	5% outliers extremos
19|	Tamanho|	xlarge_low|	3000|	30%	|nenhuma	-
20|	Tamanho|	xlarge_med|	3000	|50%	|nenhuma	-


Muito Grandes (n = 4000, 5000)	
|Categoria |	Tipo |	n |	W% |	Correlação	|Característica especial|
|-|-|-|-|-|-|					
21	|Tamanho|	xxlarge_low	|4000	|30%	|nenhuma	-
22	|Tamanho	|xxlarge_med|	4000|	50%|	nenhuma	-
23	|Estrutura|	hard_4000	|4000|	50%	|nenhuma	capacidade crítica
24	|Estrutura	|uniform_5000|	5000	|50%	|nenhuma	uniforme [1,100]
25|	Estrutura	|clustered_5000	|5000	|50%|	nenhuma	20 clusters

## Gerador de Instâncias
```bash
# Compilar
gcc -o gerar_instancias gerar_instancias.c -lm

# Gerar 1000 cópias de cada uma das 25 instâncias (total 25.000 arquivos)
./gerar_instancias 1000

# Gerar 100 cópias (total 2.500 arquivos)
./gerar_instancias 100

# Gerar 10 cópias (total 250 arquivos)
./gerar_instancias 10

# Modo simulação (mostra o que seria gerado sem criar arquivos)
./gerar_instancias 1000 --dry-run

# Ajuda
./gerar_instancias --help
```

## Estrutura
```bash
Trabalho-4---AEDs-III/
├── src/
│   ├── main.c
│   ├── Makefile
│   └── testador.c
├── instances/
│   ├── small/
│   │   └── *.txt
│   ├── medium/
│   │   └── *.txt
│   ├── large/
│   │   └── *.txt
│   └── xlarge/
│       └── *.txt
└── docs/
    ├── results.csv (gerado pelo testador)
    └── tp4 (executável compilado)
```