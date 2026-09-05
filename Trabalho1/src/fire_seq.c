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

typedef struct {
  int valor_gerado[2];
  int codigo;
  const char* ds_cobertura;
} Cobertura;

typedef struct {
  int id_cobertura;
  int fator;
} FatorIncendio;

typedef struct {
  int codigo;
  const char* ds_estado;
} CelulaEstado;

static const Cobertura TIPOS_COBERTURA[] = {
  { {0, 9}, 0, "Água (10\%)" },
  { {10, 19}, 1, "solo exposto (10\%)" },
  { {20, 54}, 2, "vegetação rasteira (35\%)" },
  { {55, 99}, 3, "Floresta (45\%) "}
};

static const FatorIncendio FATORES_INCENDIO[] = {
  { 0, 0 },
  { 1, 0 },
  { 2, 8 },
  { 3, 12 },
};

static const CelulaEstado TIPOS_ESTADO_CELULA[] = {
  {0, "não combustível"},
  {1, "intacta"},
  {2, "em chamas"},
  {3, "queimada"},
  {4, "contenção"}
};


/**
Definições de células



*/
typedef struct {
  int id_estado;
  int fator_inicio_incendio;
  int umidade;
} Celula;

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
  int L, C, P, T, SEED, LIMIAR;
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
    // TODO!

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

  Celula matrix[configs->L][configs->C];

  free_input_configs(configs);
  return EXIT_SUCCESS;
}