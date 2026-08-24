# ============================================================================
# Makefile para Trabalho Prático 4 - Heurísticas para Knapsack
# Disciplina: AEDS 3
# Data: 15/06/2026
# ============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -O2 -lm
DEBUG_FLAGS = -Wall -Wextra -g -O0 -lm -DDEBUG

TARGET = tp4
TESTADOR = testador
SRCS = main.c
TESTADOR_SRC = testador.c
OBJS = $(SRCS:.c=.o)

# Cores para output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
NC = \033[0m

# ============================================================================

all: $(TARGET) $(TESTADOR)
	@echo "$(GREEN)Compilação concluída com sucesso!$(NC)"
	@echo "$(BLUE)Executáveis: $(TARGET) e $(TESTADOR)$(NC)"

$(TARGET): $(OBJS)
	@echo "$(YELLOW)Linkando objetos...$(NC)"
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS)
	@echo "$(GREEN)✓ Executável criado: $(TARGET)$(NC)"

$(TESTADOR): $(TESTADOR_SRC)
	@echo "$(YELLOW)Compilando testador...$(NC)"
	$(CC) -o $(TESTADOR) $(TESTADOR_SRC) $(CFLAGS)
	@echo "$(GREEN)✓ Executável criado: $(TESTADOR)$(NC)"

%.o: %.c
	@echo "$(YELLOW)Compilando $<...$(NC)"
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================

debug: CFLAGS = $(DEBUG_FLAGS)
debug: clean $(TARGET) $(TESTADOR)
	@echo "$(GREEN)Modo debug ativado!$(NC)"

run: $(TARGET)
	@echo "$(BLUE)Executando tp4 com instância padrão...$(NC)"
	@echo "Uso: ./$(TARGET) <caminho_para_instancia.txt>"
	@echo "Exemplo: ./$(TARGET) ../instances/small/knapsack_tiny_low_00001.txt"

test: $(TARGET) $(TESTADOR)
	@echo "$(YELLOW)Executando testador...$(NC)"
	./$(TESTADOR)

test-quick: $(TARGET)
	@echo "$(YELLOW)Teste rápido (primeiras 5 instâncias)...$(NC)"
	@find ../instances -name "*.txt" 2>/dev/null | head -5 | while read f; do \
		echo "Processando: $$f"; \
		./$(TARGET) "$$f"; \
		echo "---"; \
	done

# ============================================================================

clean:
	@echo "$(YELLOW)Limpando arquivos temporários...$(NC)"
	rm -f $(OBJS) $(TARGET) $(TESTADOR)
	@echo "$(GREEN)✓ Limpeza concluída$(NC)"

clean-all: clean
	@echo "$(YELLOW)Removendo logs e resultados...$(NC)"
	rm -f ../docs/*.csv
	@echo "$(GREEN)✓ Limpeza completa$(NC)"

# ============================================================================

help:
	@echo "$(BLUE)========================================$(NC)"
	@echo "$(GREEN)Makefile para TP4 - Knapsack Heurísticas$(NC)"
	@echo "$(BLUE)========================================$(NC)"
	@echo ""
	@echo "$(YELLOW)Comandos disponíveis:$(NC)"
	@echo "  make          - Compila tp4 e testador"
	@echo "  make debug    - Compila versão com debug"
	@echo "  make run      - Mostra como executar tp4"
	@echo "  make test     - Executa testador em todas instâncias"
	@echo "  make test-quick - Testa apenas 5 instâncias"
	@echo "  make clean    - Remove objetos e executáveis"
	@echo "  make clean-all- Remove também resultados"
	@echo "  make help     - Mostra esta mensagem"
	@echo ""

.PHONY: all debug run test test-quick clean clean-all help