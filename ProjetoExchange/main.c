#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define MAX_USUARIOS 1000
#define ARQ_USUARIOS "usuarios.txt"

#define BTC 0
#define ETH 1
#define XRP 2

typedef struct {
    char cpf[12];
    char senha[7];
    char nome[61];
    double reais;
    double cripto[3];
} Usuario;

Usuario usuarios[MAX_USUARIOS];
int qtdUsuarios = 0;

double cotacao[3] = {350000.00, 16000.00, 3.00};

const char *NOMES[3]  = {"Bitcoin", "Ethereum", "Ripple"};
const char *SIGLAS[3] = {"BTC", "ETH", "XRP"};
const double TAXA_COMPRA[3] = {0.02, 0.01, 0.01};
const double TAXA_VENDA[3]  = {0.03, 0.02, 0.01};

void lerLinha(char *buf, int tam) {
    int len;
    if (fgets(buf, tam, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
        len--;
        if (len > 0 && buf[len - 1] == '\r') buf[len - 1] = '\0';
    } else {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
}

int lerDouble(const char *msg, double *valor) {
    char buf[64], *fim;
    int i;
    printf("%s", msg);
    lerLinha(buf, sizeof(buf));
    for (i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == ',') buf[i] = '.';
    }
    if (buf[0] == '\0') return 0;
    *valor = strtod(buf, &fim);
    if (*fim != '\0') return 0;
    return 1;
}

int lerInt(const char *msg) {
    char buf[32];
    printf("%s", msg);
    lerLinha(buf, sizeof(buf));
    if (buf[0] == '\0') return -1;
    return atoi(buf);
}

int soDigitos(const char *s, int tamanho) {
    int i;
    if ((int)strlen(s) != tamanho) return 0;
    for (i = 0; i < tamanho; i++) {
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    return 1;
}

void pausar(void) {
    char buf[8];
    printf("\nPressione ENTER para continuar...");
    lerLinha(buf, sizeof(buf));
}

void limpar(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void formatarCPF(const char *cpf, char *saida) {
    sprintf(saida, "%.3s.%.3s.%.3s-%.2s", cpf, cpf + 3, cpf + 6, cpf + 9);
}

int linhasIgnoradas = 0;

void carregarUsuarios(void) {
    FILE *f = fopen(ARQ_USUARIOS, "r");
    char linha[300];
    Usuario u;
    qtdUsuarios = 0;
    linhasIgnoradas = 0;
    if (f == NULL) return;
    while (fgets(linha, sizeof(linha), f) != NULL && qtdUsuarios < MAX_USUARIOS) {
        if (linha[0] == '\n' || linha[0] == '\r') continue;
        if (sscanf(linha, "%11[^;];%6[^;];%60[^;];%lf;%lf;%lf;%lf",
                   u.cpf, u.senha, u.nome, &u.reais,
                   &u.cripto[BTC], &u.cripto[ETH], &u.cripto[XRP]) == 7) {
            usuarios[qtdUsuarios++] = u;
        } else {
            linhasIgnoradas++;
        }
    }
    fclose(f);
}

void salvarUsuarios(void) {
    FILE *f = fopen(ARQ_USUARIOS, "w");
    int i;
    if (f == NULL) {
        printf("ERRO: nao foi possivel salvar %s (%s)\n", ARQ_USUARIOS, strerror(errno));
        printf("Dica: salve o projeto em uma pasta com permissao de escrita (ex.: Documentos ou Area de Trabalho).\n");
        return;
    }
    for (i = 0; i < qtdUsuarios; i++) {
        fprintf(f, "%s;%s;%s;%.2f;%.8f;%.8f;%.8f\n",
                usuarios[i].cpf, usuarios[i].senha, usuarios[i].nome,
                usuarios[i].reais, usuarios[i].cripto[BTC],
                usuarios[i].cripto[ETH], usuarios[i].cripto[XRP]);
    }
    fclose(f);
}

void registrarExtrato(const Usuario *u, char sinal, double valor,
                      const char *moeda, double ct, double tx) {
    char nomeArq[40];
    char data[20];
    FILE *f;
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);

    sprintf(nomeArq, "extrato_%s.txt", u->cpf);
    f = fopen(nomeArq, "a");
    if (f == NULL) {
        printf("ERRO: nao foi possivel gravar o extrato.\n");
        return;
    }
    strftime(data, sizeof(data), "%d-%m-%Y %H:%M", t);
    fprintf(f, "%s %c %.2f %s CT: %.2f TX: %.2f REAL: %.2f BTC: %.8f ETH: %.8f XRP: %.8f\n",
            data, sinal, valor, moeda, ct, tx, u->reais,
            u->cripto[BTC], u->cripto[ETH], u->cripto[XRP]);
    fclose(f);
}

int buscarUsuario(const char *cpf) {
    int i;
    for (i = 0; i < qtdUsuarios; i++) {
        if (strcmp(usuarios[i].cpf, cpf) == 0) return i;
    }
    return -1;
}

int confirmarSenha(const Usuario *u) {
    char senha[64];
    int tentativas;
    for (tentativas = 1; tentativas <= 3; tentativas++) {
        printf("Digite sua senha (6 digitos): ");
        lerLinha(senha, sizeof(senha));
        if (strcmp(senha, u->senha) == 0) return 1;
        printf("Senha incorreta! (%d/3)\n", tentativas);
    }
    return 0;
}

void mostrarCabecalho(const Usuario *u) {
    char cpfFmt[20];
    formatarCPF(u->cpf, cpfFmt);
    printf("Nome: %s\n", u->nome);
    printf("CPF: %s\n", cpfFmt);
}

void mostrarSaldos(const Usuario *u) {
    printf("Reais: %.2f\n", u->reais);
    printf("Bitcoin: %.8f\n", u->cripto[BTC]);
    printf("Ethereum: %.8f\n", u->cripto[ETH]);
    printf("Ripple: %.8f\n", u->cripto[XRP]);
}

void mostrarCotacoes(void) {
    int i;
    printf("\n--- Cotacoes atuais ---\n");
    for (i = 0; i < 3; i++) {
        printf("%d. %-9s (%s): R$ %.2f | taxa compra %.0f%% | taxa venda %.0f%%\n",
               i + 1, NOMES[i], SIGLAS[i], cotacao[i],
               TAXA_COMPRA[i] * 100, TAXA_VENDA[i] * 100);
    }
}

void cadastrarUsuario(void) {
    Usuario u;
    char cpf[64], senha[64], senha2[64];

	printf("==================================\n");
    printf("===== CADASTRAR NOVO USUARIO =====\n");
    printf("==================================\n");

    if (qtdUsuarios >= MAX_USUARIOS) {
        printf("Limite de usuarios atingido!\n");
        return;
    }

    printf("Nome completo: ");
    lerLinha(u.nome, sizeof(u.nome));
    if (strlen(u.nome) == 0 || strchr(u.nome, ';') != NULL) {
        printf("Nome invalido (nao pode ser vazio nem conter ';').\n");
        return;
    }

    printf("CPF (11 digitos, sem pontos e sem traco): ");
    lerLinha(cpf, sizeof(cpf));
    if (!soDigitos(cpf, 11)) {
        printf("CPF invalido! Digite exatamente 11 numeros.\n");
        return;
    }
    if (buscarUsuario(cpf) != -1) {
        printf("Ja existe um usuario cadastrado com esse CPF!\n");
        return;
    }

    printf("Senha numerica (6 digitos): ");
    lerLinha(senha, sizeof(senha));
    if (!soDigitos(senha, 6)) {
        printf("Senha invalida! Digite exatamente 6 numeros.\n");
        return;
    }
    printf("Confirme a senha: ");
    lerLinha(senha2, sizeof(senha2));
    if (strcmp(senha, senha2) != 0) {
        printf("As senhas nao conferem!\n");
        return;
    }

    strcpy(u.cpf, cpf);
    strcpy(u.senha, senha);
    u.reais = 0.0;
    u.cripto[BTC] = u.cripto[ETH] = u.cripto[XRP] = 0.0;

    usuarios[qtdUsuarios++] = u;
    salvarUsuarios();
    printf("\nUsuario cadastrado com sucesso! Ja e possivel fazer login.\n");
}

int efetuarLogin(void) {
    char cpf[64], senha[64];
    int idx;

	printf("=================\n");
    printf("===== LOGIN =====\n");
    printf("=================\n");
    printf("CPF (sem pontos e sem traco): ");
    lerLinha(cpf, sizeof(cpf));
    printf("Senha (6 digitos): ");
    lerLinha(senha, sizeof(senha));

    if (!soDigitos(cpf, 11)) {
        printf("CPF invalido! Digite os 11 numeros, sem pontos e sem traco.\n");
        return -1;
    }
    idx = buscarUsuario(cpf);
    if (idx == -1 || strcmp(usuarios[idx].senha, senha) != 0) {
        printf("CPF ou senha incorretos! (%d usuario(s) carregado(s) do arquivo)\n", qtdUsuarios);
        return -1;
    }
    return idx;
}

void consultarSaldo(Usuario *u) {
    printf("=================\n");
	printf("===== SALDO =====\n");
    printf("=================\n");
    if (!confirmarSenha(u)) {
        printf("Acesso negado.\n");
        return;
    }
    printf("\n");
    mostrarCabecalho(u);
    mostrarSaldos(u);
}

void consultarExtrato(Usuario *u) {
    char nomeArq[40], linha[400];
    FILE *f;
    int vazio = 1;

	printf("===================\n");
    printf("===== EXTRATO =====\n");
    printf("===================\n");
    if (!confirmarSenha(u)) {
        printf("Acesso negado.\n");
        return;
    }
    printf("\n");
    mostrarCabecalho(u);
    printf("\n");

    sprintf(nomeArq, "extrato_%s.txt", u->cpf);
    f = fopen(nomeArq, "r");
    if (f != NULL) {
        while (fgets(linha, sizeof(linha), f) != NULL) {
            printf("%s", linha);
            vazio = 0;
        }
        fclose(f);
    }
    if (vazio) printf("Nenhuma operacao realizada ainda.\n");
}

void depositar(Usuario *u) {
    double valor;
    printf("====================\n");
    printf("===== DEPOSITO =====\n");
    printf("====================\n");
    if (!lerDouble("Valor do deposito (R$): ", &valor) || valor <= 0) {
        printf("Valor invalido!\n");
        return;
    }
    u->reais += valor;
    registrarExtrato(u, '+', valor, "REAL", 0.0, 0.0);
    salvarUsuarios();
    printf("\nDeposito realizado com sucesso!\n");
    mostrarSaldos(u);
}

void sacar(Usuario *u) {
    double valor;
    printf("=================\n");
	printf("===== SAQUE =====\n");
	printf("=================\n");
    printf("Saldo disponivel: R$ %.2f\n", u->reais);
    if (!lerDouble("Valor do saque (R$): ", &valor) || valor <= 0) {
        printf("Valor invalido!\n");
        return;
    }
    if (valor > u->reais) {
        printf("Saldo insuficiente! O saldo nao pode ficar negativo.\n");
        return;
    }
    u->reais -= valor;
    registrarExtrato(u, '-', valor, "REAL", 0.0, 0.0);
    salvarUsuarios();
    printf("\nSaque realizado com sucesso!\n");
    mostrarSaldos(u);
}

void comprarCripto(Usuario *u) {
    int op;
    double valor, taxa, total, qtd;
    char resp[16];

	printf("================================\n");
    printf("===== COMPRAR CRIPTOMOEDAS =====\n");
    printf("================================\n");
    if (!confirmarSenha(u)) {
        printf("Acesso negado.\n");
        return;
    }

    mostrarCotacoes();
    printf("Saldo em reais: R$ %.2f\n\n", u->reais);
    op = lerInt("Qual criptomoeda deseja comprar? (1-3, 0 para cancelar): ");
    if (op == 0) return;
    if (op < 1 || op > 3) {
        printf("Opcao invalida!\n");
        return;
    }
    op--;

    if (!lerDouble("Quanto deseja comprar (valor em R$, sem a taxa): ", &valor) || valor <= 0) {
        printf("Valor invalido!\n");
        return;
    }

    taxa  = valor * TAXA_COMPRA[op];
    total = valor + taxa;
    qtd   = valor / cotacao[op];

    if (total > u->reais) {
        printf("Saldo insuficiente! Necessario R$ %.2f (valor + taxa), disponivel R$ %.2f.\n",
               total, u->reais);
        return;
    }

    printf("\nResumo da compra:\n");
    printf("  Moeda: %s (%s)\n", NOMES[op], SIGLAS[op]);
    printf("  Valor: R$ %.2f | Taxa (%.0f%%): R$ %.2f | Total debitado: R$ %.2f\n",
           valor, TAXA_COMPRA[op] * 100, taxa, total);
    printf("  Voce recebera: %.8f %s\n", qtd, SIGLAS[op]);
    printf("Confirmar? (s/n): ");
    lerLinha(resp, sizeof(resp));
    if (resp[0] != 's' && resp[0] != 'S') {
        printf("Compra cancelada.\n");
        return;
    }

    u->reais -= total;
    u->cripto[op] += qtd;
    registrarExtrato(u, '+', valor, SIGLAS[op], cotacao[op], taxa);
    salvarUsuarios();
    printf("\nCompra realizada com sucesso!\n");
    mostrarSaldos(u);
}

void venderCripto(Usuario *u) {
    int op;
    double qtd, bruto, taxa, liquido;
    char resp[16];

	printf("===============================\n");
    printf("===== VENDER CRIPTOMOEDAS =====\n");
    printf("===============================\n");
    if (!confirmarSenha(u)) {
        printf("Acesso negado.\n");
        return;
    }

    mostrarCotacoes();
    printf("Seus saldos: BTC %.8f | ETH %.8f | XRP %.8f\n\n",
           u->cripto[BTC], u->cripto[ETH], u->cripto[XRP]);
    op = lerInt("Qual criptomoeda deseja vender? (1-3, 0 para cancelar): ");
    if (op == 0) return;
    if (op < 1 || op > 3) {
        printf("Opcao invalida!\n");
        return;
    }
    op--;

    if (!lerDouble("Quantidade a vender: ", &qtd) || qtd <= 0) {
        printf("Quantidade invalida!\n");
        return;
    }
    if (qtd > u->cripto[op] + 0.000000001) {
        printf("Saldo insuficiente de %s! O saldo nao pode ficar negativo.\n", SIGLAS[op]);
        return;
    }

    bruto   = qtd * cotacao[op];
    taxa    = bruto * TAXA_VENDA[op];
    liquido = bruto - taxa;

    printf("\nResumo da venda:\n");
    printf("  Moeda: %s (%s)\n", NOMES[op], SIGLAS[op]);
    printf("  Valor bruto: R$ %.2f | Taxa (%.0f%%): R$ %.2f | Voce recebera: R$ %.2f\n",
           bruto, TAXA_VENDA[op] * 100, taxa, liquido);
    printf("Confirmar? (s/n): ");
    lerLinha(resp, sizeof(resp));
    if (resp[0] != 's' && resp[0] != 'S') {
        printf("Venda cancelada.\n");
        return;
    }

    u->cripto[op] -= qtd;
    if (u->cripto[op] < 0) u->cripto[op] = 0;
    u->reais += liquido;
    registrarExtrato(u, '-', bruto, SIGLAS[op], cotacao[op], taxa);
    salvarUsuarios();
    printf("\nVenda realizada com sucesso!\n");
    mostrarSaldos(u);
}

void atualizarCotacao(void) {
    int i;
    double variacao;

	printf("=============================\n");
    printf("===== ATUALIZAR COTACAO =====\n");
    printf("=============================\n");
    for (i = 0; i < 3; i++) {
        variacao = ((rand() % 10001) / 10000.0) * 0.10 - 0.05;
        printf("%-9s: R$ %.2f -> ", NOMES[i], cotacao[i]);
        cotacao[i] = cotacao[i] * (1.0 + variacao);
        printf("R$ %.2f (%+.2f%%)\n", cotacao[i], variacao * 100);
    }
}

void menuUsuario(int idx) {
    int op;
    do {
        limpar();
        printf("==========================\n");
        printf("===== MENU PRINCIPAL =====\n");
        printf("==========================\n");
        printf("Bem-vindo(a), %s!\n\n", usuarios[idx].nome);
        printf("1. Consultar saldo\n");
        printf("2. Consultar extrato\n");
        printf("3. Depositar\n");
        printf("4. Sacar\n");
        printf("5. Comprar criptomoedas\n");
        printf("6. Vender criptomoedas\n");
        printf("7. Atualizar cotacao\n");
        printf("8. Sair\n");
        op = lerInt("Escolha uma opcao: ");

        limpar();
        switch (op) {
            case 1: consultarSaldo(&usuarios[idx]);   pausar(); break;
            case 2: consultarExtrato(&usuarios[idx]); pausar(); break;
            case 3: depositar(&usuarios[idx]);        pausar(); break;
            case 4: sacar(&usuarios[idx]);            pausar(); break;
            case 5: comprarCripto(&usuarios[idx]);    pausar(); break;
            case 6: venderCripto(&usuarios[idx]);     pausar(); break;
            case 7: atualizarCotacao();               pausar(); break;
            case 8: break;
            default: printf("Opcao invalida!\n"); pausar();
        }
    } while (op != 8);
}

int main(void) {
    int op, idx;

    srand((unsigned int)time(NULL));
    carregarUsuarios();

    if (linhasIgnoradas > 0) {
        printf("AVISO: %d linha(s) de %s nao puderam ser lidas e foram ignoradas.\n",
               linhasIgnoradas, ARQ_USUARIOS);
        pausar();
    }

    do {
        limpar();
        printf("====================================\n");
        printf("===== EXCHANGE DE CRIPTOMOEDAS =====\n");
        printf("====================================\n");
        printf("1. Efetuar login\n");
        printf("2. Cadastrar novo usuario\n");
        printf("0. Encerrar programa\n");
        op = lerInt("Escolha uma opcao: ");

        limpar();
        switch (op) {
            case 1:
                idx = efetuarLogin();
                if (idx != -1) menuUsuario(idx);
                else pausar();
                break;
            case 2:
                cadastrarUsuario();
                pausar();
                break;
            case 0:
                printf("Ate logo!\n");
                break;
            default:
                printf("Opcao invalida!\n");
                pausar();
        }
    } while (op != 0);

    return 0;
}
