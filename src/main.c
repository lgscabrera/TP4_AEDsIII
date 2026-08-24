/*
 * Trabalho Prático 4 – Heurísticas para Knapsack
 * Grupo: [Nomes]
 * Data: 15/06/2026
 * Disciplina: AEDS 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// ========================== ESTRUTURAS ==========================

typedef struct {
    int n;
    int capacity;
    int *w;
    int *v;
} Instance;

// ========================== LEITURA ==========================

Instance* read_instance(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Erro ao abrir arquivo %s\n", filename);
        return NULL;
    }
    
    Instance *inst = (Instance*)malloc(sizeof(Instance));
    fscanf(f, "%d %d", &inst->n, &inst->capacity);
    
    inst->w = (int*)malloc(inst->n * sizeof(int));
    inst->v = (int*)malloc(inst->n * sizeof(int));
    
    for (int i = 0; i < inst->n; i++) {
        fscanf(f, "%d %d", &inst->w[i], &inst->v[i]);
    }
    
    fclose(f);
    return inst;
}

void free_instance(Instance *inst) {
    if (inst) {
        free(inst->w);
        free(inst->v);
        free(inst);
    }
}

// ========================== FUNÇÕES AUXILIARES ==========================

void evaluate(int *sol, Instance *inst, int *value, int *weight) {
    *value = 0;
    *weight = 0;
    for (int i = 0; i < inst->n; i++) {
        if (sol[i]) {
            *value += inst->v[i];
            *weight += inst->w[i];
        }
    }
}

void repair(int *sol, Instance *inst) {
    int weight = 0;
    for (int i = 0; i < inst->n; i++) {
        if (sol[i]) weight += inst->w[i];
    }
    
    while (weight > inst->capacity) {
        int worst_idx = -1;
        double worst_ratio = 1e9;
        for (int i = 0; i < inst->n; i++) {
            if (sol[i]) {
                double ratio = (double)inst->v[i] / inst->w[i];
                if (ratio < worst_ratio) {
                    worst_ratio = ratio;
                    worst_idx = i;
                }
            }
        }
        if (worst_idx != -1) {
            sol[worst_idx] = 0;
            weight -= inst->w[worst_idx];
        } else break;
    }
}

void greedy_solution(int *sol, Instance *inst) {
    int *idx = (int*)malloc(inst->n * sizeof(int));
    double *ratio = (double*)malloc(inst->n * sizeof(double));
    
    for (int i = 0; i < inst->n; i++) {
        idx[i] = i;
        ratio[i] = (double)inst->v[i] / inst->w[i];
    }
    
    for (int i = 0; i < inst->n - 1; i++) {
        for (int j = i + 1; j < inst->n; j++) {
            if (ratio[i] < ratio[j]) {
                int tmp_idx = idx[i];
                idx[i] = idx[j];
                idx[j] = tmp_idx;
                double tmp_r = ratio[i];
                ratio[i] = ratio[j];
                ratio[j] = tmp_r;
            }
        }
    }
    
    for (int i = 0; i < inst->n; i++) sol[i] = 0;
    int current_weight = 0;
    for (int i = 0; i < inst->n; i++) {
        int item = idx[i];
        if (current_weight + inst->w[item] <= inst->capacity) {
            sol[item] = 1;
            current_weight += inst->w[item];
        }
    }
    
    free(idx);
    free(ratio);
}

// ========================== SIMULATED ANNEALING ==========================

void simulated_annealing(Instance *inst, int *best_sol, int *best_value, double *time_spent) {
    clock_t start = clock();
    
    int n = inst->n;
    int *current = (int*)malloc(n * sizeof(int));
    int *next = (int*)malloc(n * sizeof(int));
    
    greedy_solution(current, inst);
    memcpy(best_sol, current, n * sizeof(int));
    
    double T = 1000.0;
    double T_min = 1e-3;
    double alpha = 0.995;
    int L = 100 * n;
    
    int current_val, current_w;
    evaluate(current, inst, &current_val, &current_w);
    int best_val = current_val;
    
    while (T > T_min) {
        for (int i = 0; i < L; i++) {
            memcpy(next, current, n * sizeof(int));
            int pos = rand() % n;
            next[pos] = 1 - next[pos];
            
            int next_val, next_w;
            evaluate(next, inst, &next_val, &next_w);
            
            int delta = next_val - current_val;
            
            if (delta > 0 || ((double)rand() / RAND_MAX) < exp(delta / T)) {
                memcpy(current, next, n * sizeof(int));
                current_val = next_val;
                current_w = next_w;
                
                if (current_val > best_val) {
                    best_val = current_val;
                    memcpy(best_sol, current, n * sizeof(int));
                }
            }
        }
        T *= alpha;
    }
    
    repair(best_sol, inst);
    evaluate(best_sol, inst, best_value, &current_w);
    
    *time_spent = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    free(current);
    free(next);
}

// ========================== ALGORITMO GENÉTICO CORRIGIDO ==========================

typedef struct {
    int *sol;
    int value;
    int weight;
} Individual;

int compare_individuals(const void *a, const void *b) {
    Individual *ia = (Individual*)a;
    Individual *ib = (Individual*)b;
    return ib->value - ia->value;
}

void free_individual(Individual *ind) {
    if (ind->sol) {
        free(ind->sol);
        ind->sol = NULL;
    }
}

void genetic_algorithm(Instance *inst, int *best_sol, int *best_value, double *time_spent) {
    clock_t start = clock();
    
    int n = inst->n;
    int pop_size = 50;
    int generations = 200;
    double mutation_prob = 0.01;
    int elite_count = 5;
    
    // Alocar população
    Individual *pop = (Individual*)malloc(pop_size * sizeof(Individual));
    Individual *new_pop = (Individual*)malloc(pop_size * sizeof(Individual));
    
    // Inicializar
    for (int i = 0; i < pop_size; i++) {
        pop[i].sol = (int*)malloc(n * sizeof(int));
        new_pop[i].sol = NULL;
        
        if (i < pop_size / 5) {
            greedy_solution(pop[i].sol, inst);
        } else {
            for (int j = 0; j < n; j++) {
                pop[i].sol[j] = rand() % 2;
            }
            repair(pop[i].sol, inst);
        }
        evaluate(pop[i].sol, inst, &pop[i].value, &pop[i].weight);
    }
    
    // Melhor solução global
    int *global_best = (int*)malloc(n * sizeof(int));
    int global_best_value = 0;
    
    for (int gen = 0; gen < generations; gen++) {
        qsort(pop, pop_size, sizeof(Individual), compare_individuals);
        
        if (pop[0].value > global_best_value) {
            global_best_value = pop[0].value;
            memcpy(global_best, pop[0].sol, n * sizeof(int));
        }
        
        // Elitismo
        for (int i = 0; i < elite_count; i++) {
            if (!new_pop[i].sol) {
                new_pop[i].sol = (int*)malloc(n * sizeof(int));
            }
            memcpy(new_pop[i].sol, pop[i].sol, n * sizeof(int));
            new_pop[i].value = pop[i].value;
            new_pop[i].weight = pop[i].weight;
        }
        
        // Cruzamento
        for (int i = elite_count; i < pop_size; i++) {
            int p1 = rand() % (pop_size / 2);
            int p2 = rand() % (pop_size / 2);
            
            if (!new_pop[i].sol) {
                new_pop[i].sol = (int*)malloc(n * sizeof(int));
            }
            
            int point = rand() % (n - 1) + 1;
            for (int j = 0; j < point; j++) {
                new_pop[i].sol[j] = pop[p1].sol[j];
            }
            for (int j = point; j < n; j++) {
                new_pop[i].sol[j] = pop[p2].sol[j];
            }
            
            for (int j = 0; j < n; j++) {
                if ((double)rand() / RAND_MAX < mutation_prob) {
                    new_pop[i].sol[j] = 1 - new_pop[i].sol[j];
                }
            }
            
            repair(new_pop[i].sol, inst);
            evaluate(new_pop[i].sol, inst, &new_pop[i].value, &new_pop[i].weight);
        }
        
        // Trocar populações
        for (int i = 0; i < pop_size; i++) {
            free_individual(&pop[i]);
        }
        
        memcpy(pop, new_pop, pop_size * sizeof(Individual));
        
        for (int i = 0; i < pop_size; i++) {
            new_pop[i].sol = NULL;
        }
    }
    
    qsort(pop, pop_size, sizeof(Individual), compare_individuals);
    if (pop[0].value > global_best_value) {
        global_best_value = pop[0].value;
        memcpy(global_best, pop[0].sol, n * sizeof(int));
    }
    
    memcpy(best_sol, global_best, n * sizeof(int));
    *best_value = global_best_value;
    *time_spent = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    // Liberar memória
    for (int i = 0; i < pop_size; i++) {
        free_individual(&pop[i]);
        if (new_pop[i].sol) free_individual(&new_pop[i]);
    }
    free(pop);
    free(new_pop);
    free(global_best);
}

// ========================== FUNÇÃO PRINCIPAL ==========================

void extract_instance_info(const char *filename, char *type, int *size) {
    const char *basename = strrchr(filename, '/');
    if (basename) {
        basename++;
    } else {
        basename = filename;
    }
    
    char temp[256];
    strcpy(temp, basename);
    char *dot = strrchr(temp, '.');
    if (dot) *dot = '\0';
    
    char *last_underscore = strrchr(temp, '_');
    if (last_underscore) {
        *size = atoi(last_underscore + 1);
        *last_underscore = '\0';
    } else {
        *size = 0;
    }
    
    if (strncmp(temp, "knapsack_", 9) == 0) {
        strcpy(type, temp + 9);
    } else {
        strcpy(type, temp);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <arquivo_instancia.txt>\n", argv[0]);
        return 1;
    }
    
    srand(time(NULL));
    
    char instance_type[100];
    int instance_size;
    extract_instance_info(argv[1], instance_type, &instance_size);
    
    printf("========================================\n");
    printf("Problema: Knapsack (0/1)\n");
    printf("Instância: %s\n", argv[1]);
    printf("Tipo: %s\n", instance_type);
    printf("Tamanho (n): %d\n", instance_size);
    printf("========================================\n\n");
    
    Instance *inst = read_instance(argv[1]);
    if (!inst) {
        return 1;
    }
    
    int *sa_sol = (int*)malloc(inst->n * sizeof(int));
    int sa_value;
    double sa_time;
    
    printf("--- SIMULATED ANNEALING ---\n");
    simulated_annealing(inst, sa_sol, &sa_value, &sa_time);
    printf("Valor encontrado: %d\n", sa_value);
    printf("Tempo de execução: %.4f segundos\n", sa_time);
    printf("Memória utilizada (aproximada): %zu bytes\n", (size_t)(3 * inst->n * sizeof(int)));
    
    int *ga_sol = (int*)malloc(inst->n * sizeof(int));
    int ga_value;
    double ga_time;
    
    printf("\n--- ALGORITMO GENÉTICO ---\n");
    genetic_algorithm(inst, ga_sol, &ga_value, &ga_time);
    printf("Valor encontrado: %d\n", ga_value);
    printf("Tempo de execução: %.4f segundos\n", ga_time);
    printf("Memória utilizada (aproximada): %zu bytes\n\n", (size_t)(100 * inst->n * sizeof(int)));
    
    printf("--- COMPARAÇÃO ---\n");
    printf("Melhor valor: SA = %d, GA = %d\n", sa_value, ga_value);
    printf("Mais rápido: %s\n", (sa_time < ga_time) ? "SA" : "GA");
    printf("Mais eficiente em memória: %s\n", 
           (3 * inst->n < 100 * inst->n) ? "SA" : "GA");
    
    free(sa_sol);
    free(ga_sol);
    free_instance(inst);
    
    return 0;
}