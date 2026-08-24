#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

// ========================== ESTRUTURAS ==========================

typedef struct {
    int n;
    int capacity;
    int *w;
    int *v;
} Instance;

// ========================== FUNÇÕES AUXILIARES ==========================

int random_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Gera instância base com pesos e valores
void generate_base_weights_values(int n, int *w, int *v, int correlation_type) {
    // Pesos sempre entre 1 e 100
    for (int i = 0; i < n; i++) {
        w[i] = random_int(1, 100);
    }
    
    // Valores baseado na correlação
    switch(correlation_type) {
        case 0: // Não correlacionado
            for (int i = 0; i < n; i++) {
                v[i] = random_int(1, 100);
            }
            break;
        case 1: // Correlação positiva forte
            for (int i = 0; i < n; i++) {
                v[i] = w[i] + random_int(1, 50);
            }
            break;
        case 2: // Correlação negativa forte
            for (int i = 0; i < n; i++) {
                v[i] = 150 - w[i] + random_int(1, 30);
                if (v[i] < 1) v[i] = 1;
            }
            break;
        default:
            for (int i = 0; i < n; i++) {
                v[i] = random_int(1, 100);
            }
    }
}

// Aplica estrutura de clusters
void apply_clusters(int n, int *w, int *v, int num_clusters) {
    int cluster_size = n / num_clusters;
    for (int c = 0; c < num_clusters; c++) {
        int base_w = 10 + (c % 10) * 15;
        int base_v = 20 + (c % 10) * 25;
        for (int i = c * cluster_size; i < (c + 1) * cluster_size && i < n; i++) {
            w[i] = base_w + random_int(-5, 15);
            v[i] = base_v + random_int(-10, 20);
            if (w[i] < 1) w[i] = 1;
            if (v[i] < 1) v[i] = 1;
        }
    }
}

// Aplica outliers (itens com valor extremamente alto)
void apply_outliers(int n, int *w, int *v, double outlier_percent) {
    int n_outliers = (int)(n * outlier_percent);
    if (n_outliers < 1) n_outliers = 1;
    
    // Embaralhar índices para outliers não ficarem sempre no início
    int *indices = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) indices[i] = i;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    
    for (int i = 0; i < n_outliers; i++) {
        int idx = indices[i];
        w[idx] = random_int(1, 20);       // peso pequeno
        v[idx] = random_int(500, 2000);    // valor muito alto
    }
    
    free(indices);
}

// Calcula peso total
int total_weight(int n, int *w) {
    int sum = 0;
    for (int i = 0; i < n; i++) sum += w[i];
    return sum;
}

// Gera capacidade crítica (difícil)
int critical_capacity(int n, int *w, int *v) {
    // Ordena itens por valor/peso (simulação simples)
    typedef struct {
        int idx;
        double ratio;
    } Item;
    
    Item *items = (Item*)malloc(n * sizeof(Item));
    for (int i = 0; i < n; i++) {
        items[i].idx = i;
        items[i].ratio = (double)v[i] / w[i];
    }
    
    // Ordenação simples
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (items[i].ratio < items[j].ratio) {
                Item tmp = items[i];
                items[i] = items[j];
                items[j] = tmp;
            }
        }
    }
    
    // Soma pesos dos melhores itens até ~60% do total
    int total_w = total_weight(n, w);
    int cap = 0;
    for (int i = 0; i < n && cap < total_w * 0.6; i++) {
        cap += w[items[i].idx];
    }
    
    free(items);
    return cap;
}

// ========================== GERADOR PRINCIPAL ==========================

void generate_single_instance(const char *filename, int n, int capacity_percent, 
                               int correlation_type, int special_type, int num_clusters) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Erro ao criar arquivo %s\n", filename);
        return;
    }
    
    int *w = (int*)malloc(n * sizeof(int));
    int *v = (int*)malloc(n * sizeof(int));
    
    // Gerar base
    generate_base_weights_values(n, w, v, correlation_type);
    
    // Aplicar estruturas especiais
    if (special_type == 1) { // Clusters
        apply_clusters(n, w, v, num_clusters);
    }
    else if (special_type == 2) { // Outliers
        apply_outliers(n, w, v, 0.05); // 5% outliers
    }
    else if (special_type == 3) { // Instância difícil (capacidade crítica)
        // Mantém pesos/valores, capacidade será calculada separadamente
    }
    
    // Calcular capacidade
    int total_w = total_weight(n, w);
    int capacity;
    
    if (special_type == 3) { // Capacidade crítica
        capacity = critical_capacity(n, w, v);
        capacity_percent = (int)((double)capacity / total_w * 100);
    } else {
        capacity = (total_w * capacity_percent) / 100;
    }
    
    // Escrever arquivo
    fprintf(f, "%d %d\n", n, capacity);
    for (int i = 0; i < n; i++) {
        fprintf(f, "%d %d\n", w[i], v[i]);
    }
    
    fclose(f);
    free(w);
    free(v);
}

// Estrutura para definir um tipo de instância
typedef struct {
    const char *base_name;
    int n;
    int capacity_percent;
    int correlation_type;
    int special_type;
    int num_clusters;
} InstanceType;

// Lista das 25 configurações de instância
InstanceType instances[] = {
    // Categoria 1: Tamanhos pequenos (n ≤ 200)
    {"knapsack_tiny_low", 50, 30, 0, 0, 0},
    {"knapsack_tiny_med", 50, 50, 0, 0, 0},
    {"knapsack_tiny_high", 50, 70, 0, 0, 0},
    {"knapsack_small_low", 100, 30, 0, 0, 0},
    {"knapsack_small_med", 100, 50, 0, 0, 0},
    {"knapsack_small_high", 100, 70, 0, 0, 0},
    {"knapsack_medium_low", 200, 30, 0, 0, 0},
    {"knapsack_medium_med", 200, 50, 0, 0, 0},
    {"knapsack_medium_high", 200, 70, 0, 0, 0},
    
    // Categoria 2: Tamanhos médios (n = 500, 1000)
    {"knapsack_large_low", 500, 30, 0, 0, 0},
    {"knapsack_large_med", 500, 50, 0, 0, 0},
    {"knapsack_large_high", 500, 70, 0, 0, 0},
    {"knapsack_uncorr", 1000, 50, 0, 0, 0},
    {"knapsack_poscorr", 1000, 50, 1, 0, 0},
    {"knapsack_negcorr", 1000, 50, 2, 0, 0},
    
    // Categoria 3: Tamanhos grandes (n = 2000, 3000)
    {"knapsack_uniform", 2000, 50, 0, 0, 0},
    {"knapsack_clustered", 2000, 50, 0, 1, 10},
    {"knapsack_outlier", 2000, 50, 0, 2, 0},
    {"knapsack_xlarge_low", 3000, 30, 0, 0, 0},
    {"knapsack_xlarge_med", 3000, 50, 0, 0, 0},
    
    // Categoria 4: Tamanhos muito grandes (n = 4000, 5000)
    {"knapsack_xxlarge_low", 4000, 30, 0, 0, 0},
    {"knapsack_xxlarge_med", 4000, 50, 0, 0, 0},
    {"knapsack_hard", 4000, 50, 0, 3, 0},
    {"knapsack_uniform_5000", 5000, 50, 0, 0, 0},
    {"knapsack_clustered_5000", 5000, 50, 0, 1, 20}
};

#define NUM_INSTANCE_TYPES 25

// ========================== FUNÇÃO PARA CRIAR DIRETÓRIO ==========================

#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(dir) _mkdir(dir)
#else
    #include <sys/stat.h>
    #define MKDIR(dir) mkdir(dir, 0777)
#endif

void create_directory(const char *dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        MKDIR(dir);
    }
}

// ========================== FUNÇÃO PRINCIPAL ==========================

void print_usage(const char *program_name) {
    printf("Uso: %s <numero_de_copias>\n", program_name);
    printf("Exemplo: %s 1000\n", program_name);
    printf("  Gera 1000 cópias de cada uma das 25 instâncias\n");
    printf("  Total de arquivos gerados: 25.000\n\n");
    printf("Opções:\n");
    printf("  --help     Mostra esta ajuda\n");
    printf("  --dry-run  Mostra quantos arquivos seriam gerados sem criá-los\n");
}

int main(int argc, char *argv[]) {
    int num_copies = 0;
    int dry_run = 0;
    
    // Parse arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = 1;
        } else if (argv[i][0] != '-') {
            num_copies = atoi(argv[i]);
        }
    }
    
    if (num_copies <= 0) {
        printf("Erro: número de cópias deve ser maior que 0\n");
        print_usage(argv[0]);
        return 1;
    }
    
    printf("========================================\n");
    printf("Gerador de %d cópias de 25 instâncias\n", num_copies);
    printf("Total de arquivos: %d\n", num_copies * NUM_INSTANCE_TYPES);
    printf("========================================\n\n");
    
    if (dry_run) {
        printf("[DRY RUN] Modo simulação - nenhum arquivo será criado\n");
        printf("Arquivos que seriam gerados:\n");
        for (int i = 0; i < NUM_INSTANCE_TYPES; i++) {
            printf("  %s_*.txt (%d cópias)\n", instances[i].base_name, num_copies);
        }
        printf("\nTotal: %d arquivos\n", num_copies * NUM_INSTANCE_TYPES);
        return 0;
    }
    
    // Criar diretórios organizados por tamanho
    create_directory("instances");
    create_directory("instances/small");
    create_directory("instances/medium");
    create_directory("instances/large");
    create_directory("instances/xlarge");
    
    srand(time(NULL));
    
    int total_generated = 0;
    clock_t start_time = clock();
    
    for (int inst_idx = 0; inst_idx < NUM_INSTANCE_TYPES; inst_idx++) {
        InstanceType *inst = &instances[inst_idx];
        
        // Determinar subdiretório baseado no tamanho
        const char *subdir;
        if (inst->n <= 200) subdir = "instances/small";
        else if (inst->n <= 1000) subdir = "instances/medium";
        else if (inst->n <= 3000) subdir = "instances/large";
        else subdir = "instances/xlarge";
        
        printf("Gerando %d cópias de %s (n=%d)... ", num_copies, inst->base_name, inst->n);
        fflush(stdout);
        
        for (int copy = 0; copy < num_copies; copy++) {
            // Nome do arquivo com número sequencial (5 dígitos)
            char filename[512];
            snprintf(filename, sizeof(filename), "%s/%s_%05d.txt", 
                     subdir, inst->base_name, copy + 1);
            
            generate_single_instance(filename, 
                                     inst->n, 
                                     inst->capacity_percent,
                                     inst->correlation_type,
                                     inst->special_type,
                                     inst->num_clusters);
            total_generated++;
            
            // Mostrar progresso a cada 10%
            if (total_generated % (num_copies * NUM_INSTANCE_TYPES / 10) == 0) {
                int percent = (total_generated * 100) / (num_copies * NUM_INSTANCE_TYPES);
                printf("\rProgresso: %d%% (%d/%d arquivos)", 
                       percent, total_generated, num_copies * NUM_INSTANCE_TYPES);
                fflush(stdout);
            }
        }
        printf(" OK\n");
    }
    
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("\n========================================\n");
    printf("✓ Geração concluída!\n");
    printf("  Total de arquivos: %d\n", total_generated);
    printf("  Tempo total: %.2f segundos\n", elapsed);
    printf("  Média: %.2f arquivos/segundo\n", total_generated / elapsed);
    printf("========================================\n");
    printf("\nEstrutura de diretórios:\n");
    printf("  instances/small/     - Instâncias com n ≤ 200\n");
    printf("  instances/medium/    - Instâncias com n ≤ 1000\n");
    printf("  instances/large/     - Instâncias com n ≤ 3000\n");
    printf("  instances/xlarge/    - Instâncias com n ≤ 5000\n");
    
    return 0;
}