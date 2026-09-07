#include <stdio.h>
#include <stdlib.h>

/**
 * Definições de foco de incêndio
 * - Coordenadas de célula em uma matriz
 * 
 * =================
 * L: linha (matrix)
 * C: coluna (matrix)
 */
typedef struct {
  int L, C;
} FocoIncendio ;

/**
 * Definições de zonas de contenção
 * - Coordenadas de retângulo (limites inclusos) em uma matriz
 * 
 * =================
 * PASSO_ATIVACAO: instante de ativação
 * LI: linha inicial
 * CI: coluna inicial
 * LF: linha final
 * CF: coluna final
 */
typedef struct {
  int PASSO_ATIVACAO;
  int LI, CI, LF, CF;
} ZonaContencao;

/**
 * Definições de célula
 * 
 * ====================
 * ID_ESTADO: Código do estado da célula
 * ID_COBERTURA: Código da cobertura da celula
 * FATOR_INCENDIO: Fator para inicio de incendios, relativo ao tipo de cobertura
 * UMIDADE: Valor de umidade da celula
 */
typedef struct {
  int ID_ESTADO;
  int ID_COBERTURA;
  int FATOR_INCENDIO;
  int UMIDADE;
} Celula;


/**
 * Vetor de tempo com métricas
 * 
 * ===========================
 * PASSO: Valor incremental
 * NAO_COMBUSTIVEIS: total de células em estado 0 (não combustível)
 * COMBUSTIVEIS: total de celulas com cobertura 2 ou 3 (cobertura de vegestação rasteira ou floresta)
 * INTACTAS: total de células em estado 1 (intacta)
 * EM_CHAMAS: total de células em estado 2 (em chamas)
 * QUEIMADAS: total de células em estado 3 (queimada)
 * CONTENCAO: total de células em estado 4 (contenção)
 * TOTAL_IGNICOES: total de ignoções ocorridas no passo atual
 * PERCENTUAL_QUEIMADO: perc. de queimadas em relação ao estado inicial
 * PERCENTUAL_PROTEGIDO: perc. de celulas de contenção em relação aos combustiveis no estado inicial
 */
typedef struct {
  int PASSO;
  int NAO_COMBUSTIVEIS;
  int INTACTAS;
  int EM_CHAMAS;
  int QUEIMADAS;
  int CONTENCAO;
  int TOTAL_IGNICOES;
  float PERCENTUAL_QUEIMADO;
  float PERCENTUAL_PROTEGIDO;
} Metrics;

/**
 * Definições de entrada
 * 
 * ============================
 * L: número de linhas (matrix)
 * C: número de colunas (matrix)
 * P: número máximo de passos para a simulação
 * T: número de threads (caso paralelo)
 * SEED: semente pseudoaleatoria
 * LIMIAR: potencial mínimo de ignição
 * 
 * ============================
 * VENTO_LINHA: componente vertical do vento
 * VENTO_COLUNA: componente horizontal do vento
 * VENTO: intensidade do vento
 * 
 * ============================
 * F: focos inuiciais de incêndio
 * Z: quantidade de zonas de contenção
 * FOCOS_INCENDIO: focos de incêndio
 * ZONAS_CONTENCAO: zonas de contenção
 */
struct input_configs {
  unsigned int L, C, P, T, SEED, LIMIAR;
  int VENTO_LINHA, VENTO_COLUNA, V;
  int F, Z;
  FocoIncendio** FOCOS_INCENDIO;
  ZonaContencao** ZONAS_CONTENCAO;
};


// Definições de funções
// =====================

// Carregar e liberar memória
struct input_configs* load_input_configs(const char* filepath);
void free_input_configs(struct input_configs* configs);

// Impressão de configurações (debug)
void print_loaded_input_configs(struct input_configs* configs);

// Validação de dados
bool is_valid_input_configs(struct input_configs* configs);

bool is_valid_single_input_argument(int argc);
bool is_valid_configs_first_line(struct input_configs* configs);
bool is_valid_configs_vento(struct input_configs* configs);
bool is_valid_F_Z(struct input_configs* configs);
bool is_valid_matrix_focos_incendio(struct input_configs* configs);
bool is_valid_matrix_zonas_contencao(struct input_configs* configs);
bool is_valid_foco_incendio_sobre_celula_combustivel(struct input_configs* configs, int id_cobertura, int i, int j);

// Geração de cobertura


bool is_valid_configs_first_line(struct input_configs* configs) {
  // Validação 3: Argumentos de primeira linha

  // Validação 3.1: L > 0
  if (configs->L <= 0) {
    printf("L > 0. Informado: L = %d\n", configs->L);
    return false;
  }

  // Validação 3.2: C > 0
  if (configs->C <= 0) {
    printf("C > 0. Informado: C = %d\n", configs->C);
    return false;
  }

  // Validação 3.3: P >= 0
  if (configs->P < 0) {
    printf("P >= 0. Informado: P = %d\n", configs->P);
    return false;
  }

  // Validação 3.4: T >= 0
  if (configs->T < 0) {
    printf("T >= 0. Informado: T = %d\n", configs->T);
    return false;
  }

  // Validação 3.5: LIMIAR > 0
  if (configs->LIMIAR <= 0) {
    printf("LIMIAR > 0. Informado: LIMIAR = %d\n", configs->LIMIAR);
    return false;
  }

  return true;
}

struct input_configs* load_input_configs(const char* filepath) {
  FILE* file = fopen(filepath, "r");

  // Validação 2: Abertura de arquivo de entrada
  if (!file) {
    perror("Erro ao abrir arquivo\n");
    return NULL;
  }

  struct input_configs* configs = (struct input_configs*) malloc(sizeof(struct input_configs));
  if (!configs) {
    perror("Erro ao armazenar estrutura de configuração\n");
    fclose(file);
    free(configs);
    return NULL;
  }

  int fscanf_return;
  // Configuração: linha 1
  // L, C, P, T, SEED, LIMIAR
  fscanf_return = fscanf(
    file, 
    "%d %d %d %d %d %d", 
    &configs->L, &configs->C, &configs->P, &configs->T,
    &configs->SEED, &configs->LIMIAR
  );

  if (fscanf_return !=6) {
    perror("Erro em leitura de parâmetros de arquivo de entrada. Configurações da linha 1\n");
    fclose(file);
    free(configs);
    return NULL;
  }

  // Configuração: linha 2
  // VENTO_LINHA, VENTO_COLUNA, V
  fscanf_return = fscanf(
    file,
    "%d %d %d",
    &configs->VENTO_LINHA, &configs->VENTO_COLUNA, &configs->V
  );

  if (fscanf_return != 3) {
    perror("Erro em leitura de parâmetros de arquivo de entrada. Configurações da linha 2\n");
    fclose(file);
    free(configs);
    return NULL;
  }

  // Configuração: linha 3
  // F, Z
  fscanf_return = fscanf(
    file,
    "%d %d",
    &configs->F, &configs->Z
  );

  if (fscanf_return != 2) {
    perror("Erro em leitura de parâmetros de arquivo de entrada. Configurações da linha 3\n");
    fclose(file);
    free(configs);
    return NULL;
  }

  // Validação prévia à alocação de memória
  if (!is_valid_F_Z(configs)) {
    fclose(file);
    free(configs);
    return NULL;
  }

  // Configuração: Linhas de focos de incêndio 
  configs->FOCOS_INCENDIO = (FocoIncendio**) malloc(configs->F * sizeof(FocoIncendio*));
  for (int i = 0; i < configs->F; i++) {
    configs->FOCOS_INCENDIO[i] = (FocoIncendio*) malloc(sizeof(FocoIncendio));
  }
  
  for (int i = 0; i < configs->F; i++) {
    // Coordenadas de Focos: L, C
    fscanf_return = fscanf(
      file, 
      "%d %d",
      &configs->FOCOS_INCENDIO[i]->L,
      &configs->FOCOS_INCENDIO[i]->C
    );

    if (fscanf_return != 2) {
      perror("Erro em leitura de parâmetros de arquivo de entrada. Configurações da coordenadas de focos de incêndio\n");
      fclose(file);
      free_input_configs(configs);
      return NULL;
    }
  }

  configs->ZONAS_CONTENCAO = (ZonaContencao**) malloc(configs->Z * sizeof(ZonaContencao*));
  for (int i = 0; i < configs->Z; i++) {
    configs->ZONAS_CONTENCAO[i] =  (ZonaContencao*) malloc(sizeof(ZonaContencao));
  }

  for (int i = 0; i < configs->Z; i++) { 
    // Coordenadas de Zonas: PASSO_ATIVACAO, LI, CI, LF, CF
    fscanf_return = fscanf(
      file,
      "%d %d %d %d %d",
      &configs->ZONAS_CONTENCAO[i]->PASSO_ATIVACAO,
      &configs->ZONAS_CONTENCAO[i]->LI,
      &configs->ZONAS_CONTENCAO[i]->CI,
      &configs->ZONAS_CONTENCAO[i]->LF,
      &configs->ZONAS_CONTENCAO[i]->CF
    );

    if (fscanf_return != 5) {
      perror("Erro em leitura de parâmetros de arquivo de entrada. Configurações da coordenadas de zonas de contenção\n");
      fclose(file);
      free_input_configs(configs);
      return NULL;
    }
  }

  fclose(file);
  return configs;
}

void free_input_configs(struct input_configs* configs) {
  if (!configs) return;

  if (configs->FOCOS_INCENDIO) {
    for (int i = 0; i < configs->F; i++) {
      free(configs->FOCOS_INCENDIO[i]);
    }
    free(configs->FOCOS_INCENDIO);
  }

  if (configs->ZONAS_CONTENCAO) {
    for (int i = 0; i < configs->Z; i++) {
      free(configs->ZONAS_CONTENCAO[i]);
    }
    free(configs->ZONAS_CONTENCAO);
  }

  free(configs);
}

void print_loaded_input_configs(struct input_configs* configs) {
  printf("=== Input Configuration Loaded ===\n");
  printf("Matrix (LxC): %dx%d | Steps (P): %d | Threads (T): %d | Seed : %d | Limiar: %d\n",
          configs->L, configs->C, configs->P, configs->T, configs->SEED, configs->LIMIAR);
  printf("Vento (direção L, C): [%d, %d] V: %d\n",
          configs->VENTO_LINHA, configs->VENTO_COLUNA, configs->V);
  printf("Focos (F = %d):\n", configs->F);
  for (int i = 0; i < configs->F; i++) {
      printf("  - Foco %d (L, C): %d, %d\n", i + 1, configs->FOCOS_INCENDIO[i]->L, configs->FOCOS_INCENDIO[i]->C);
  }
  printf("Zonas (Z = %d):\n", configs->Z);
  for (int i = 0; i < configs->Z; i++) {
      printf("  - Zona %d: Passo=%d, Range (LI, CI): [%d, %d] (LF, CF): [%d, %d]\n",
        i + 1,
        configs->ZONAS_CONTENCAO[i]->PASSO_ATIVACAO,
        configs->ZONAS_CONTENCAO[i]->LI, configs->ZONAS_CONTENCAO[i]->CI,
        configs->ZONAS_CONTENCAO[i]->LF, configs->ZONAS_CONTENCAO[i]->CF
      );
  }
}

bool is_valid_F_Z(struct input_configs* configs) {
  // Validação 5: Focos e Zonas

  // Validação 5.1: F >= 0
  if (configs->F < 0) {
    printf("F >= 0. Informado: F = %d\n", configs->F);
    return false;
  }

  // Validação 5.2: Z >= 0
  if (configs->Z < 0) {
    printf("Z >= 0. Informado: Z = %d\n", configs->Z);
    return false;
  }

  return true;
}

bool is_valid_matrix_focos_incendio(struct input_configs* configs) {
  // Validação 6: Focos

  if (configs->F == 0) return true;

  int L = configs->FOCOS_INCENDIO[0]->L;
  int C = configs->FOCOS_INCENDIO[0]->C;

  for (int i = 0; i < configs->F; i++) {
    // Validação 6.1: Focos dentro da matriz
    if (
      (configs->FOCOS_INCENDIO[i]->L < 0 || configs->FOCOS_INCENDIO[i]->C < 0) ||
      (configs->FOCOS_INCENDIO[i]->L >= configs->L || configs->FOCOS_INCENDIO[i]->C >= configs->C)
    ) {
      printf(
        "Focos de incêndio devem estar dentro da matriz. Informado: (%d, %d)\n", 
        configs->FOCOS_INCENDIO[i]->L, configs->FOCOS_INCENDIO[i]->C
      );
      return false;
    }

    // Validação 6.2: Ausência de focos repetidos
    if (i != 0 && (configs->FOCOS_INCENDIO[i]->L == L && configs->FOCOS_INCENDIO[i]->C == C)) {
      printf(
        "Focos não devem estar duplicados. Informado: (%d, %d)\n", 
        configs->FOCOS_INCENDIO[i]->L, configs->FOCOS_INCENDIO[i]->C
      );
      return false;
    }

    // Validação 6.3: Focos sobre células de combustivels
    // Realizada após gerar matriz

    L = configs->FOCOS_INCENDIO[i]->L;
    C = configs->FOCOS_INCENDIO[i]->C;
  }

  return true;
}

bool is_valid_matrix_zonas_contencao(struct input_configs* configs) {
  // Validação 7: Zonas de contenção

  if (configs->Z == 0) return true;

  for (int i = 0; i < configs->Z; i++) {
    // Validação 7.1: Zonas dentro da matriz
    if (
      (configs->ZONAS_CONTENCAO[i]->LI < 0 || configs->ZONAS_CONTENCAO[i]->CI < 0) ||
      (configs->ZONAS_CONTENCAO[i]->LI >= configs->L || configs->ZONAS_CONTENCAO[i]->CI >= configs->C)
    ) {
      printf(
        "Zonas de contenção devem estar dentro da matriz. Informado: (%d, %d)\n", 
        configs->ZONAS_CONTENCAO[i]->LI, configs->ZONAS_CONTENCAO[i]->CI
      );
      return false;
    }

    if (
      (configs->ZONAS_CONTENCAO[i]->LF < 0 || configs->ZONAS_CONTENCAO[i]->CF < 0) ||
      (configs->ZONAS_CONTENCAO[i]->LF >= configs->L || configs->ZONAS_CONTENCAO[i]->CF >= configs->C)
    ) {
      printf(
        "Zonas de contenção devem estar dentro da matriz. Informado: (%d, %d)\n", 
        configs->ZONAS_CONTENCAO[i]->LF, configs->ZONAS_CONTENCAO[i]->CF
      );
      return false;
    }

    // Validação 7.2: Limites iniciais < Limites finais
    if (
      (configs->ZONAS_CONTENCAO[i]->LI > configs->ZONAS_CONTENCAO[i]->LF) || 
      (configs->ZONAS_CONTENCAO[i]->CI > configs->ZONAS_CONTENCAO[i]->CF)
    ) {
      printf(
        "(LI, CI) < (LF, CF). Informado: (%d, %d), (%d, %d)\n", 
        configs->ZONAS_CONTENCAO[i]->LI, configs->ZONAS_CONTENCAO[i]->CI,
        configs->ZONAS_CONTENCAO[i]->LF, configs->ZONAS_CONTENCAO[i]->CF
      );
      return false;
    }

    // Validação 8: Passo de ativação
    if (configs->ZONAS_CONTENCAO[i]->PASSO_ATIVACAO < 0 || configs->ZONAS_CONTENCAO[i]->PASSO_ATIVACAO >= configs->P) {
      printf(
        "0 <= PASSO_ATIVACAO < P . Informado: %d\n", 
        configs->ZONAS_CONTENCAO[i]->PASSO_ATIVACAO
      );
      return false;
    }
  }

  return true;
}

bool is_valid_configs_vento(struct input_configs* configs) {
  // Validação 4: Vento

  // Validação 4.1: -1 <= VENTO_LINHA <= 1 e -1 <= VENTO_COLUNA <= 1
  if (
      (configs->VENTO_LINHA < -1 || configs->VENTO_LINHA > 1) || 
      (configs->VENTO_COLUNA < -1 || configs->VENTO_COLUNA > 1)
    ) { 
      printf("-1 <= VENTO_LINHA <= 1 e -1 <= VENTO_COLUNA <= 1. Informado: (L,C) = (%d,%d)\n", configs->VENTO_LINHA, configs->VENTO_COLUNA);
      return false;
  }
  
  // Validação 4.2: (VENTO_LINHA, VENTO_COLUNA) != (0,0)
  if (configs->VENTO_LINHA == 0 && configs->VENTO_COLUNA == 0){ 
    printf("(VENTO_LINHA, VENTO_COLUNA) != (0,0). Informado: (L,C) = (%d,%d)\n", configs->VENTO_LINHA, configs->VENTO_COLUNA);
    return false;
  }

  // Validação 4.3: 0 <= V <= 5
  if (configs->V < 0 || configs->V > 5) {
    printf("0 <= V <= 5. Informado: V = %d\n", configs->V);
    return false;
  }

  return true;
}

bool is_valid_input_configs(struct input_configs* configs) {
  return is_valid_configs_first_line(configs) &&
    is_valid_configs_vento(configs) &&
    is_valid_matrix_focos_incendio(configs) && 
    is_valid_matrix_zonas_contencao(configs);
}

bool is_valid_single_input_argument(int argc) {
  return argc == 2 ? true : false;
}

/**
  { {0, 9}, 0, "Água (10\%)" },
  { {10, 19}, 1, "solo exposto (10\%)" },
  { {20, 54}, 2, "vegetação rasteira (35\%)" },
  { {55, 99}, 3, "Floresta (45\%) "}
  */
int generate_cobertura(struct input_configs* configs) {
  int valor = rand_r(&configs->SEED) % 100;
  
  if (valor <= 9) return 0;
  if (valor <= 19) return 1;
  if (valor <= 54) return 2;
  
  return 3;
}

/**
  { 0, 0 },
  { 1, 0 },
  { 2, 8 },
  { 3, 12 },
*/
int generate_fator_incendio(int id_cobertura) {
  if (id_cobertura == 0 || id_cobertura == 1) return 0;
  if (id_cobertura == 2) return 8;
  return 12;
}

int generate_umidade(struct input_configs* configs) {
  return rand_r(&configs->SEED) % 101;
}

int generate_estado(int id_cobertura) {
  if (id_cobertura == 0 || id_cobertura == 1) return 0;
  return 1;
}

int calculate_linear_matrix_index(int row, int col, int C) {
  return (int) row * C + col;
}

bool is_valid_foco_incendio_sobre_celula_combustivel(struct input_configs* configs, int id_cobertura, int i, int j) {
  // Validação 6.3: Focos sobre células de combustivels

  if (id_cobertura == 2 || id_cobertura == 3) return true;
  
  for (int k = 0; k < configs->F; k++) {
    if (configs->FOCOS_INCENDIO[k]->L == i && configs->FOCOS_INCENDIO[k]->C == j) {
        printf("Focos posicionados sobre células combustíveis. Informado: (L,C) = (%d,%d)\n", i, j);
        return false;
    }
  }

  return true;
}

bool populate_matrix(struct input_configs* configs, Celula *matrix) {
  if (!configs || !matrix) return false;


  for (int i = 0; i < configs->L; i++) {
    for (int j = 0; j < configs->C; j++) {
      int idx = calculate_linear_matrix_index(i, j, configs->C);
      
      int id_cobertura = generate_cobertura(configs);
      
      matrix[idx].ID_COBERTURA = id_cobertura;
      matrix[idx].FATOR_INCENDIO = generate_fator_incendio(id_cobertura);
      matrix[idx].UMIDADE = generate_umidade(configs);
      matrix[idx].ID_ESTADO = generate_estado(id_cobertura);

      if (!is_valid_foco_incendio_sobre_celula_combustivel(configs, id_cobertura, i, j)) return false;
    }
  }

  return true;
}

void print_state_matrix(struct input_configs* configs, Celula* matrix) {
  for (int i = 0; i < configs->L; i++) {
    for (int j = 0; j < configs->C; j++) {
      int idx = calculate_linear_matrix_index(i, j, configs->C);
      printf("%d ", matrix[idx].ID_ESTADO);
    }
    printf("\n");
  }
}

void free_simulation_matrix(struct input_configs* configs, Celula* matrix) {
  if (!matrix) return;
  free(matrix);
}

Celula *build_linear_state_matrix(struct input_configs* configs) {
  Celula *matrix = (Celula*) malloc((size_t) configs->L * configs->C * sizeof(Celula));
  if (!matrix) return NULL;

  return matrix;
}


Metrics *build_metrics_vector(struct input_configs* configs) {
  Metrics *vector = (Metrics*) malloc(configs->P * sizeof(Metrics));
  if (!vector) return NULL;
  return vector;
}

void free_metrics_vector(Metrics *vector) {
  if (!vector) return;
  free(vector);
}

void free_mapa_contencao_vector(int *vector) {
  if (!vector) return;
  free(vector);
}

void apply_focos_iniciais_incendio(struct input_configs* configs, Celula* matrix) {
  for (int i = 0; i < configs->F; i++) {
    int idx = calculate_linear_matrix_index(configs->FOCOS_INCENDIO[i]->L, configs->FOCOS_INCENDIO[i]->C, configs->C);
    printf("%d -> %d %d\n", idx, configs->FOCOS_INCENDIO[i]->L, configs->FOCOS_INCENDIO[i]->C);
    matrix[idx].ID_ESTADO = 2;
  }
}

int get_passo_ativacao_if_cell_in_zona_contencao(int row, int col, struct input_configs* configs) {
  // Obter mínimo caso célula pertencer a zonas de contenção sobrepostas
  // max(passo) = P-1
  int min_passo_ativacao = configs->P;
  for (int i = 0; i < configs->Z; i++) {
    if (
      (row >= configs->ZONAS_CONTENCAO[i]->LI && row <= configs->ZONAS_CONTENCAO[i]->LF) &&
      (col >= configs->ZONAS_CONTENCAO[i]->CI && col <= configs->ZONAS_CONTENCAO[i]->CF)
    ) min_passo_ativacao = configs->ZONAS_CONTENCAO[i]->PASSO_ATIVACAO < min_passo_ativacao ? configs->ZONAS_CONTENCAO[i]->PASSO_ATIVACAO : min_passo_ativacao;
  }
  return min_passo_ativacao == configs->P ? -1 : min_passo_ativacao;
}

int *build_mapa_contencao(struct input_configs* configs, Celula* matrix) {
  int *ativacao = (int*) malloc((size_t) configs->L * configs->C * sizeof(int));
  if (!ativacao) return NULL;

  for (int i = 0; i < configs->L; i++) {
    for (int j = 0; j < configs->C; j++) {
      int idx = calculate_linear_matrix_index(i, j, configs->C);
      ativacao[idx] = get_passo_ativacao_if_cell_in_zona_contencao(i, j, configs);
    }
  }
  return ativacao;
}

void activate_zonas_contencao(struct input_configs* configs, Celula* matrix, int p) {
  for (int i = 0; i < configs->Z; i++) {
    if (configs->ZONAS_CONTENCAO[i]->PASSO_ATIVACAO != p) continue;
    
    for (int l = configs->ZONAS_CONTENCAO[i]->LI; l <= configs->ZONAS_CONTENCAO[i]->LF; l++) {
      for (int c = configs->ZONAS_CONTENCAO[i]->CI; c <= configs->ZONAS_CONTENCAO[i]->CF; c++) {
        int idx = calculate_linear_matrix_index(l, c, configs->C);
        if (matrix[idx].ID_ESTADO != 1) continue;
        matrix[idx].ID_ESTADO = 4;
      }
    }
  }
}

void run_simulation(
  struct input_configs* configs,
  Celula *matrix_estado_atual,
  Celula *matrix_proximo_estado,
  Metrics *vetor_tempo_atual,
  Metrics *vetor_proximo_tempo
) {
  for (int p = 0; p < configs->P; p++) {
    activate_zonas_contencao(configs, matrix_estado_atual, p);
  }

}


int main(int argc, char* argv[]) {
  // Validação 1: Presença de uym único argumento
  if (!is_valid_single_input_argument(argc)) {
    perror("Entrada inválida. A execução do programa requer um argumento: ./program <input_configs_filename>\n");
    return EXIT_FAILURE;
  }

  struct input_configs* configs = load_input_configs(argv[1]);
  if (!configs) {
    perror("Arquivo de configurações inválido.\n");
    free_input_configs(configs);
    return EXIT_FAILURE;
  }

  if (!is_valid_input_configs(configs)) {
    perror("Arquivo de configurações inválido.\n");
    free_input_configs(configs);
    return EXIT_FAILURE;
  }

  print_loaded_input_configs(configs);

  Celula *matrix_estado_atual = build_linear_state_matrix(configs);
  Celula *matrix_proximo_estado = build_linear_state_matrix(configs);
  Metrics *vetor_tempo_atual = build_metrics_vector(configs);
  Metrics *vetor_proximo_tempo = build_metrics_vector(configs);
  int *vetor_ativacao = build_mapa_contencao(configs, matrix_estado_atual);

  if (!matrix_estado_atual || !matrix_proximo_estado || !vetor_tempo_atual || !vetor_proximo_tempo || !vetor_ativacao) {
    perror("Falha ao alocar memória para matriz");
    free_simulation_matrix(configs, matrix_estado_atual);
    free_simulation_matrix(configs, matrix_proximo_estado);
    free_metrics_vector(vetor_tempo_atual);
    free_metrics_vector(vetor_proximo_tempo);
    free_mapa_contencao_vector(vetor_ativacao);
    free_input_configs(configs);
    return EXIT_FAILURE;
  }

  if (!populate_matrix(configs, matrix_estado_atual) || !populate_matrix(configs, matrix_proximo_estado)) {
    perror("Falha ao popular matriz.\n");
    free_simulation_matrix(configs, matrix_estado_atual);
    free_simulation_matrix(configs, matrix_proximo_estado);
    free_metrics_vector(vetor_tempo_atual);
    free_metrics_vector(vetor_proximo_tempo);
    free_mapa_contencao_vector(vetor_ativacao);
    free_input_configs(configs);
    return EXIT_FAILURE;
  }

  apply_focos_iniciais_incendio(configs, matrix_estado_atual);
  print_state_matrix(configs, matrix_estado_atual);

  free_simulation_matrix(configs, matrix_estado_atual);
  free_simulation_matrix(configs, matrix_proximo_estado);
  free_metrics_vector(vetor_tempo_atual);
  free_metrics_vector(vetor_proximo_tempo);
  free_mapa_contencao_vector(vetor_ativacao);
  free_input_configs(configs);
  return EXIT_SUCCESS;
}