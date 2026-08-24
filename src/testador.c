/*
 * Testador para Trabalho Prático 4 - Knapsack Heurísticas
 * 
 * Compilar: gcc -o testador testador.c
 * Executar: ./testador
 * 
 * Este programa varre todas as instâncias na pasta ../instances/,
 * executa o tp4 para cada uma e salva os resultados em ../docs/results.csv
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#ifdef _WIN32
    #include <direct.h>
    #define PATH_SEP '\\'
    #define MKDIR(dir) _mkdir(dir)
#else
    #include <sys/types.h>
    #define PATH_SEP '/'
    #define MKDIR(dir) mkdir(dir, 0777)
#endif

// ========================== CONSTANTES ==========================

#define MAX_PATH 1024
#define MAX_LINE 4096
#define TP4_EXECUTABLE "./tp4"                    // Executável na mesma pasta
#define INSTANCES_DIR "../instances"              // Instâncias um nível acima
#define RESULTS_DIR "../docs"                     // Resultados um nível acima
#define RESULTS_FILE "../docs/results.csv"
#define TIMEOUT_SECONDS 300

// ========================== ESTRUTURAS ==========================

typedef struct {
    char filename[MAX_PATH];
    char relative_path[MAX_PATH];
    char instance_type[100];
    int size;
    int sa_value;
    double sa_time;
    size_t sa_memory;
    int ga_value;
    double ga_time;
    size_t ga_memory;
    int error;
    char error_msg[256];
} TestResult;

// ========================== FUNÇÕES AUXILIARES ==========================

// Cria diretório se não existir
void create_directory(const char *dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        MKDIR(dir);
        printf("  Criado diretório: %s\n", dir);
    }
}

// Extrai informações do nome do arquivo
void extract_instance_info(const char *filename, char *type, int *size) {
    char basename[MAX_PATH];
    strcpy(basename, filename);
    
    // Remove extensão
    char *dot = strrchr(basename, '.');
    if (dot) *dot = '\0';
    
    // Remove o número sequencial no final (ex: _00001)
    char *last_underscore = strrchr(basename, '_');
    if (last_underscore) {
        *last_underscore = '\0';
        char *size_pos = strrchr(basename, '_');
        if (size_pos) {
            *size = atoi(size_pos + 1);
        } else {
            *size = 0;
        }
    }
    
    strcpy(type, basename);
    
    // Remove o prefixo "knapsack_" para ficar mais limpo
    if (strncmp(type, "knapsack_", 9) == 0) {
        memmove(type, type + 9, strlen(type) - 8);
    }
}

// Executa comando e captura saída
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    char buffer[MAX_LINE];
    output[0] = '\0';
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(output, buffer, output_size - strlen(output) - 1);
    }
    
    int ret = pclose(fp);
    return ret;
}

// Processa a saída do tp4 e preenche a estrutura TestResult
void parse_output(const char *output, TestResult *result) {
    result->sa_value = 0;
    result->sa_time = 0.0;
    result->sa_memory = 0;
    result->ga_value = 0;
    result->ga_time = 0.0;
    result->ga_memory = 0;
    
    char *line = strtok((char*)output, "\n");
    int sa_section = 0;
    int ga_section = 0;
    
    while (line) {
        if (strstr(line, "SIMULATED ANNEALING")) {
            sa_section = 1;
            ga_section = 0;
        }
        else if (strstr(line, "ALGORITMO GENÉTICO")) {
            sa_section = 0;
            ga_section = 1;
        }
        else if (strstr(line, "Valor encontrado:") && sa_section) {
            sscanf(line, "Valor encontrado: %d", &result->sa_value);
        }
        else if (strstr(line, "Tempo de execução:") && sa_section) {
            sscanf(line, "Tempo de execução: %lf segundos", &result->sa_time);
        }
        else if (strstr(line, "Memória utilizada") && sa_section) {
            sscanf(line, "Memória utilizada (aproximada): %zu bytes", &result->sa_memory);
        }
        else if (strstr(line, "Valor encontrado:") && ga_section) {
            sscanf(line, "Valor encontrado: %d", &result->ga_value);
        }
        else if (strstr(line, "Tempo de execução:") && ga_section) {
            sscanf(line, "Tempo de execução: %lf segundos", &result->ga_time);
        }
        else if (strstr(line, "Memória utilizada") && ga_section) {
            sscanf(line, "Memória utilizada (aproximada): %zu bytes", &result->ga_memory);
        }
        
        line = strtok(NULL, "\n");
    }
}

// Executa tp4 para uma instância
int run_tp4(const char *instance_path, TestResult *result) {
    char cmd[MAX_PATH * 2];
    char output[MAX_LINE * 10];
    
    strncpy(result->filename, instance_path, MAX_PATH - 1);
    result->filename[MAX_PATH - 1] = '\0';
    result->error = 0;
    result->error_msg[0] = '\0';
    
    snprintf(cmd, sizeof(cmd), "%s \"%s\"", TP4_EXECUTABLE, instance_path);
    
    int ret = execute_and_capture(cmd, output, sizeof(output));
    
    if (ret != 0) {
        result->error = 1;
        snprintf(result->error_msg, sizeof(result->error_msg), 
                 "Erro na execução (código %d)", ret);
        return -1;
    }
    
    if (strstr(output, "timeout") || strstr(output, "killed")) {
        result->error = 1;
        strcpy(result->error_msg, "Timeout ou processo morto");
        return -1;
    }
    
    parse_output(output, result);
    return 0;
}

// ========================== PERCORRE DIRETÓRIOS ==========================

int is_instance_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext && strcmp(ext, ".txt") == 0) {
        return 1;
    }
    return 0;
}

int scan_directory(const char *dir_path, TestResult **results, int *count, int max_results) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char full_path[MAX_PATH];
    int local_count = 0;
    
    dir = opendir(dir_path);
    if (dir == NULL) {
        printf("  Erro: Não foi possível abrir o diretório %s\n", dir_path);
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (stat(full_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                local_count += scan_directory(full_path, results, count, max_results);
            } 
            else if (S_ISREG(statbuf.st_mode) && is_instance_file(entry->d_name)) {
                if (*count < max_results) {
                    TestResult *result = &(*results)[*count];
                    strncpy(result->relative_path, full_path, MAX_PATH - 1);
                    result->relative_path[MAX_PATH - 1] = '\0';
                    
                    printf("  [%d] Processando: %s\n", *count + 1, full_path);
                    
                    if (run_tp4(full_path, result) == 0) {
                        printf("      SA: %d (%.3fs), GA: %d (%.3fs)\n", 
                               result->sa_value, result->sa_time,
                               result->ga_value, result->ga_time);
                    } else {
                        printf("      ERRO: %s\n", result->error_msg);
                    }
                    
                    (*count)++;
                    local_count++;
                } else {
                    printf("  Aviso: Limite máximo de resultados (%d) atingido\n", max_results);
                    break;
                }
            }
        }
    }
    
    closedir(dir);
    return local_count;
}

// ========================== SALVAR RESULTADOS ==========================

void save_results_to_csv(TestResult *results, int count) {
    create_directory(RESULTS_DIR);
    
    FILE *f = fopen(RESULTS_FILE, "w");
    if (!f) {
        printf("Erro: Não foi possível criar %s\n", RESULTS_FILE);
        return;
    }
    
    fprintf(f, "Arquivo,Tipo,Tamanho,");
    fprintf(f, "SA_Valor,SA_Tempo(s),SA_Memoria(bytes),");
    fprintf(f, "GA_Valor,GA_Tempo(s),GA_Memoria(bytes),");
    fprintf(f, "Melhor_Valor,Melhor_Algoritmo,Erro\n");
    
    for (int i = 0; i < count; i++) {
        TestResult *r = &results[i];
        
        char instance_type[100];
        int instance_size;
        extract_instance_info(r->relative_path, instance_type, &instance_size);
        
        char melhor[20] = "Empate";
        if (!r->error) {
            if (r->sa_value > r->ga_value) strcpy(melhor, "SA");
            else if (r->ga_value > r->sa_value) strcpy(melhor, "GA");
            else strcpy(melhor, "Empate");
        } else {
            strcpy(melhor, "Erro");
        }
        
        fprintf(f, "\"%s\",", r->relative_path);
        fprintf(f, "\"%s\",", instance_type);
        fprintf(f, "%d,", instance_size);
        
        if (r->error) {
            fprintf(f, "0,0,0,0,0,0,\"%s\",\"%s\"\n", melhor, r->error_msg);
        } else {
            fprintf(f, "%d,%.6f,%zu,", r->sa_value, r->sa_time, r->sa_memory);
            fprintf(f, "%d,%.6f,%zu,", r->ga_value, r->ga_time, r->ga_memory);
            fprintf(f, "\"%s\",", melhor);
            fprintf(f, "OK\n");
        }
    }
    
    fclose(f);
    printf("\n✓ Resultados salvos em: %s\n", RESULTS_FILE);
}

// ========================== RELATÓRIO RESUMIDO ==========================

void print_summary(TestResult *results, int count) {
    int success_count = 0;
    int sa_better = 0, ga_better = 0, tie = 0;
    double total_sa_time = 0, total_ga_time = 0;
    int max_sa_value = 0, max_ga_value = 0;
    char best_sa_instance[MAX_PATH] = "", best_ga_instance[MAX_PATH] = "";
    
    for (int i = 0; i < count; i++) {
        if (!results[i].error) {
            success_count++;
            total_sa_time += results[i].sa_time;
            total_ga_time += results[i].ga_time;
            
            if (results[i].sa_value > max_sa_value) {
                max_sa_value = results[i].sa_value;
                strcpy(best_sa_instance, results[i].relative_path);
            }
            if (results[i].ga_value > max_ga_value) {
                max_ga_value = results[i].ga_value;
                strcpy(best_ga_instance, results[i].relative_path);
            }
            
            if (results[i].sa_value > results[i].ga_value) sa_better++;
            else if (results[i].ga_value > results[i].sa_value) ga_better++;
            else tie++;
        }
    }
    
    printf("\n========================================\n");
    printf("RESUMO DOS TESTES\n");
    printf("========================================\n");
    printf("Total de instâncias: %d\n", count);
    printf("Execuções bem-sucedidas: %d\n", success_count);
    printf("Falhas: %d\n", count - success_count);
    printf("\n");
    printf("Comparação SA vs GA:\n");
    printf("  SA venceu: %d (%.1f%%)\n", sa_better, 
           success_count > 0 ? 100.0 * sa_better / success_count : 0);
    printf("  GA venceu: %d (%.1f%%)\n", ga_better,
           success_count > 0 ? 100.0 * ga_better / success_count : 0);
    printf("  Empate: %d (%.1f%%)\n", tie,
           success_count > 0 ? 100.0 * tie / success_count : 0);
    printf("\n");
    printf("Tempo total:\n");
    printf("  SA: %.2f segundos (média: %.3fs)\n", total_sa_time, 
           success_count > 0 ? total_sa_time / success_count : 0);
    printf("  GA: %.2f segundos (média: %.3fs)\n", total_ga_time,
           success_count > 0 ? total_ga_time / success_count : 0);
    printf("\n");
    printf("Melhor valor encontrado:\n");
    printf("  SA: %d (em %s)\n", max_sa_value, best_sa_instance);
    printf("  GA: %d (em %s)\n", max_ga_value, best_ga_instance);
    printf("========================================\n");
}

// ========================== FUNÇÃO PRINCIPAL ==========================

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("Testador de Heurísticas para Knapsack\n");
    printf("Trabalho Prático 4 - AEDS 3\n");
    printf("========================================\n\n");
    
    // Verificar se o executável tp4 existe
    if (access(TP4_EXECUTABLE, F_OK) != 0) {
        printf("ERRO: Executável '%s' não encontrado!\n", TP4_EXECUTABLE);
        printf("Certifique-se de compilar o main.c primeiro:\n");
        printf("  make\n");
        return 1;
    }
    
    if (access(TP4_EXECUTABLE, X_OK) != 0) {
        printf("ERRO: Executável '%s' não tem permissão de execução!\n", TP4_EXECUTABLE);
        printf("Execute: chmod +x %s\n", TP4_EXECUTABLE);
        return 1;
    }
    
    // Verificar se o diretório de instâncias existe
    if (access(INSTANCES_DIR, F_OK) != 0) {
        printf("ERRO: Diretório '%s' não encontrado!\n", INSTANCES_DIR);
        printf("Certifique-se de que o gerador de instâncias foi executado.\n");
        printf("Execute na pasta instances: ./gerar_instancias 10\n");
        return 1;
    }
    
    int max_results = 50000;
    TestResult *results = (TestResult*)malloc(max_results * sizeof(TestResult));
    if (!results) {
        printf("ERRO: Não foi possível alocar memória para os resultados\n");
        return 1;
    }
    
    int result_count = 0;
    
    printf("Iniciando teste de todas as instâncias em '%s/'\n\n", INSTANCES_DIR);
    
    clock_t start_time = clock();
    
    scan_directory(INSTANCES_DIR, &results, &result_count, max_results);
    
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("\n========================================\n");
    printf("Testes concluídos em %.2f segundos\n", elapsed);
    printf("Total de instâncias processadas: %d\n", result_count);
    printf("========================================\n");
    
    if (result_count > 0) {
        save_results_to_csv(results, result_count);
        print_summary(results, result_count);
    } else {
        printf("\nNenhuma instância foi processada!\n");
        printf("Verifique se o diretório '%s' contém arquivos .txt\n", INSTANCES_DIR);
    }
    
    free(results);
    
    return 0;
}