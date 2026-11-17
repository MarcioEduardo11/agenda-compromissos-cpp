// Agenda de compromissos no terminal (arquivo unico).
// Compilar: g++ -std=c++17 -Wall -Wextra -pedantic -o agenda src/main.cpp
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

// ---------- Estruturas de dados ----------

struct Compromisso {
    int id = 0;
    string nome;
    int dia = 0, mes = 0, ano = 0, hora = 0, minuto = 0;
    string local;
};

struct NoLista {  // lista encadeada: todos os compromissos
    Compromisso dados;
    NoLista* proximo;
};
struct NoPilha {  // pilha: lixeira dos excluidos (desfazer)
    Compromisso dados;
    NoPilha* proximo;
};
struct NoFila {  // fila: lembretes
    Compromisso dados;
    NoFila* proximo;
};
struct Fila {
    NoFila* inicio = nullptr;  // proximo a ser lembrado
    NoFila* fim = nullptr;     // ultimo que entrou
};
struct NoArvore {  // arvore: indice por nome (aponta para o compromisso na lista)
    Compromisso* compromisso;
    NoArvore* esquerda;
    NoArvore* direita;
};

// Reune tudo. Vai por referencia (&) para as funcoes alterarem o original.
struct Agenda {
    NoLista* lista = nullptr;
    int total = 0;
    int proximoId = 1;
    NoPilha* lixeira = nullptr;
    Fila lembretes;
    NoArvore* raiz = nullptr;
};

// ---------- Apoio: chaves, texto e exibicao ----------

// Chave AAAAMMDD: comparar duas chaves = comparar duas datas.
long long chaveData(const Compromisso& c) {
    return (long long)c.ano * 10000 + c.mes * 100 + c.dia;
}

// Chave AAAAMMDDHHMM: data e horario juntos.
long long chaveDataHora(const Compromisso& c) {
    return chaveData(c) * 10000 + c.hora * 100 + c.minuto;
}

// Devolve o texto em minuscula (so ASCII). 't' vai POR VALOR: a funcao altera a copia.
string minuscula(string t) {
    for (size_t i = 0; i < t.size(); i++)
        if (t[i] >= 'A' && t[i] <= 'Z') t[i] += 32;
    return t;
}

// Compara dois textos sem diferenciar maiusculas. Retorno: <0, 0 ou >0.
int comparar(const string& a, const string& b) {
    return minuscula(a).compare(minuscula(b));
}

// Mostra um compromisso em uma linha.
void exibir(const Compromisso& c) {
    printf("  [%d] %02d/%02d/%04d %02d:%02d | %s | %s\n", c.id, c.dia, c.mes, c.ano,
           c.hora, c.minuto, c.nome.c_str(), c.local.c_str());
}

// ---------- Entrada de dados ----------

// Le uma linha; com obrigatorio = true (valor padrao) repete ate nao ser vazia.
// Retorno: a linha ("" se a entrada terminar).
string lerTexto(const string& rotulo, bool obrigatorio = true) {
    string linha;
    cout << rotulo << ": ";
    while (getline(cin, linha)) {
        if (!linha.empty() || !obrigatorio) return linha;
        cout << "  Campo obrigatorio.\n" << rotulo << ": ";
    }
    return "";
}

// Le um inteiro de minimo a maximo. 'fim' e ponteiro (*) preenchido por strtol: se nao
// apontar para o '\0' final, a linha tinha lixo (ex.: "12abc"). Se a entrada acabar, devolve minimo.
int lerInteiro(const string& rotulo, int minimo, int maximo) {
    string linha;
    cout << rotulo << ": ";
    while (getline(cin, linha)) {
        char* fim = nullptr;
        long valor = strtol(linha.c_str(), &fim, 10);
        if (!linha.empty() && *fim == '\0' && valor >= minimo && valor <= maximo) return (int)valor;
        cout << "  Valor invalido (de " << minimo << " a " << maximo << ").\n" << rotulo << ": ";
    }
    return minimo;
}

// Pergunta sim/nao. Retorno: true so para 's' ou 'S'.
bool confirmar(const string& pergunta) {
    string resposta = lerTexto(pergunta + " (s/n)");
    return !resposta.empty() && (resposta[0] == 's' || resposta[0] == 'S');
}

// Quantidade de dias do mes naquele ano (considera ano bissexto).
int diasNoMes(int mes, int ano) {
    if (mes == 2) return ((ano % 4 == 0 && ano % 100 != 0) || ano % 400 == 0) ? 29 : 28;
    return (mes == 4 || mes == 6 || mes == 9 || mes == 11) ? 30 : 31;
}

// Le dia, mes e ano ate formarem uma data que existe. 'c' por referencia (&): preenche o original.
void lerData(Compromisso& c) {
    do {
        c.dia = lerInteiro("Dia", 1, 31);
        c.mes = lerInteiro("Mes", 1, 12);
        c.ano = lerInteiro("Ano", 2000, 2100);
        if (c.dia > diasNoMes(c.mes, c.ano)) cout << "  Essa data nao existe.\n";
    } while (c.dia > diasNoMes(c.mes, c.ano));
}

// Le hora (0-23) e minuto (0-59) para dentro de 'c'.
void lerHorario(Compromisso& c) {
    c.hora = lerInteiro("Hora", 0, 23);
    c.minuto = lerInteiro("Minuto", 0, 59);
}

// ---------- Lista encadeada [R3, R4, R5] ----------

// Insere no FIM da lista. O no e criado com new (liberado em removerPorId / liberarLista).
void inserirNoFim(Agenda& a, const Compromisso& c) {
    NoLista* novo = new NoLista{c, nullptr};
    if (a.lista == nullptr) {
        a.lista = novo;
    } else {
        NoLista* ultimo = a.lista;
        while (ultimo->proximo != nullptr) ultimo = ultimo->proximo;
        ultimo->proximo = novo;
    }
    a.total++;
}

// Cadastra: 'c' vai POR VALOR, entao o ID e preenchido na copia. Retorno: o ID criado.
int cadastrar(Agenda& a, Compromisso c) {
    c.id = a.proximoId++;
    inserirNoFim(a, c);
    return c.id;
}

// Travessia da lista procurando pelo ID. Retorno: o no ou nullptr.
NoLista* buscarPorId(const Agenda& a, int id) {
    NoLista* atual = a.lista;
    while (atual != nullptr && atual->dados.id != id) atual = atual->proximo;
    return atual;
}

// Remove o no do ID dado: 'anterior' acompanha 'atual' para religar a lista antes do delete.
// Retorno: false se o ID nao existe.
bool removerPorId(Agenda& a, int id) {
    NoLista* anterior = nullptr;
    NoLista* atual = a.lista;
    while (atual != nullptr && atual->dados.id != id) {
        anterior = atual;
        atual = atual->proximo;
    }
    if (atual == nullptr) return false;
    if (anterior == nullptr) a.lista = atual->proximo;
    else anterior->proximo = atual->proximo;
    delete atual;
    atual = nullptr;
    a.total--;
    return true;
}

// Recursivo [R3]: tamanho = 0 se o no for nulo; senao 1 + tamanho do resto da lista.
int contarNos(const NoLista* no) {
    if (no == nullptr) return 0;
    return 1 + contarNos(no->proximo);
}

// Apaga todos os nos da lista.
void liberarLista(Agenda& a) {
    while (a.lista != nullptr) {
        NoLista* proximo = a.lista->proximo;
        delete a.lista;
        a.lista = proximo;
    }
    a.total = 0;
}

// ---------- Pilha (LIFO): lixeira para desfazer exclusao [R4, R6] ----------

// push: o novo no aponta para o antigo topo e vira o topo. 'topo' por referencia (&) porque muda.
void empilhar(NoPilha*& topo, const Compromisso& c) {
    topo = new NoPilha{c, topo};
}

// pop: remove o topo (o ultimo excluido) e entrega a copia por referencia. Retorno: false se vazia.
bool desempilhar(NoPilha*& topo, Compromisso& saida) {
    if (topo == nullptr) return false;
    NoPilha* removido = topo;
    saida = removido->dados;
    topo = removido->proximo;
    delete removido;
    removido = nullptr;
    return true;
}

// ---------- Fila (FIFO): lembretes [R4, R7] ----------

// enqueue: insere no FIM da fila. Retorno: false se o compromisso ja esta na fila.
bool enfileirar(Fila& f, const Compromisso& c) {
    for (const NoFila* n = f.inicio; n != nullptr; n = n->proximo)
        if (n->dados.id == c.id) return false;
    NoFila* novo = new NoFila{c, nullptr};
    if (f.fim == nullptr) f.inicio = novo;
    else f.fim->proximo = novo;
    f.fim = novo;
    return true;
}

// dequeue: remove o PRIMEIRO (quem entrou antes). Se esvaziar, 'fim' tambem volta a nullptr.
// Retorno: false se vazia.
bool desenfileirar(Fila& f, Compromisso& saida) {
    if (f.inicio == nullptr) return false;
    NoFila* removido = f.inicio;
    saida = removido->dados;
    f.inicio = removido->proximo;
    if (f.inicio == nullptr) f.fim = nullptr;
    delete removido;
    removido = nullptr;
    return true;
}

// ---------- Arvore binaria de busca por nome [R2, R3, R8] ----------
// Por que arvore: o percurso in-ordem da os nomes em ordem alfabetica sem ordenar nada.

// Insere (recursivo): menor vai para a esquerda; igual ou maior, para a direita.
// Retorno: a raiz da subarvore depois da insercao.
NoArvore* inserirNaArvore(NoArvore* raiz, Compromisso* c) {
    if (raiz == nullptr) return new NoArvore{c, nullptr, nullptr};
    if (comparar(c->nome, raiz->compromisso->nome) < 0)
        raiz->esquerda = inserirNaArvore(raiz->esquerda, c);
    else
        raiz->direita = inserirNaArvore(raiz->direita, c);
    return raiz;
}

// Busca recursiva por nome exato. 'comparacoes' e ponteiro (*): todas as chamadas somam no
// mesmo contador. Retorno: quantos compromissos tem esse nome (repetidos ficam a direita).
int buscarNaArvore(const NoArvore* raiz, const string& nome, long* comparacoes) {
    if (raiz == nullptr) return 0;
    (*comparacoes)++;
    int cmp = comparar(nome, raiz->compromisso->nome);
    if (cmp < 0) return buscarNaArvore(raiz->esquerda, nome, comparacoes);
    if (cmp == 0) exibir(*raiz->compromisso);
    return (cmp == 0 ? 1 : 0) + buscarNaArvore(raiz->direita, nome, comparacoes);
}

// Percurso in-ordem (esquerda, raiz, direita) = ordem alfabetica. '*restante' limita quantos mostrar.
void percorrerEmOrdem(const NoArvore* raiz, int* restante) {
    if (raiz == nullptr || *restante <= 0) return;
    percorrerEmOrdem(raiz->esquerda, restante);
    if (*restante <= 0) return;
    exibir(*raiz->compromisso);
    (*restante)--;
    percorrerEmOrdem(raiz->direita, restante);
}

// Libera a arvore em pos-ordem (filhos antes do pai). Os compromissos continuam na lista.
void liberarArvore(NoArvore*& raiz) {
    if (raiz == nullptr) return;
    liberarArvore(raiz->esquerda);
    liberarArvore(raiz->direita);
    delete raiz;
    raiz = nullptr;
}

// Refaz a arvore a partir da lista (ela guarda ponteiros para a lista, entao nao pode ficar velha).
void reconstruirArvore(Agenda& a) {
    liberarArvore(a.raiz);
    for (NoLista* no = a.lista; no != nullptr; no = no->proximo)
        a.raiz = inserirNaArvore(a.raiz, &no->dados);
}

// ---------- Ordenacao [R3, R4, R9] ----------

// Junta duas metades ja ordenadas v[inicio..meio] e v[meio+1..fim] por data e horario.
// Usa um vetor auxiliar (new[] / delete[]). Em empate pega a esquerda primeiro: e estavel.
void mesclar(Compromisso** v, int inicio, int meio, int fim, long* comparacoes) {
    Compromisso** aux = new Compromisso*[fim - inicio + 1];
    int i = inicio, j = meio + 1, k = 0;
    while (i <= meio && j <= fim) {
        (*comparacoes)++;
        if (chaveDataHora(*v[i]) <= chaveDataHora(*v[j])) aux[k++] = v[i++];
        else aux[k++] = v[j++];
    }
    while (i <= meio) aux[k++] = v[i++];
    while (j <= fim) aux[k++] = v[j++];
    for (k = 0; k < fim - inicio + 1; k++) v[inicio + k] = aux[k];
    delete[] aux;
    aux = nullptr;
}

// Merge Sort (recursivo): divide ao meio, ordena cada metade e junta.
// Complexidade (n = compromissos): melhor, medio e pior caso O(n log n);
// memoria extra O(n); estavel. Ordena so ponteiros (Compromisso**), sem copiar compromissos.
void mergeSort(Compromisso** v, int inicio, int fim, long* comparacoes) {
    if (inicio >= fim) return;
    int meio = inicio + (fim - inicio) / 2;
    mergeSort(v, inicio, meio, comparacoes);
    mergeSort(v, meio + 1, fim, comparacoes);
    mesclar(v, inicio, meio, fim, comparacoes);
}

// Monta um vetor com os ENDERECOS dos compromissos e ordena. Quem chama faz delete[].
// Retorno: o vetor ordenado, ou nullptr se a agenda estiver vazia.
Compromisso** montarVetorOrdenado(const Agenda& a, long* comparacoes) {
    if (a.total == 0) return nullptr;
    Compromisso** v = new Compromisso*[a.total];
    int i = 0;
    for (NoLista* no = a.lista; no != nullptr; no = no->proximo) v[i++] = &no->dados;
    mergeSort(v, 0, a.total - 1, comparacoes);
    return v;
}

// ---------- Busca [R10] ----------

// Busca sequencial pela data (AAAAMMDD): O(1) no melhor caso, O(n) no medio e pior.
// Retorno: indice da primeira ocorrencia ou -1.
int buscaSequencial(Compromisso** v, int n, long long chave, long* comparacoes) {
    for (int i = 0; i < n; i++) {
        (*comparacoes)++;
        if (chaveData(*v[i]) == chave) return i;
    }
    return -1;
}

// Busca binaria pela data em vetor ORDENADO: O(log n). Ao achar, continua a esquerda
// para devolver a PRIMEIRA ocorrencia do dia. Retorno: indice ou -1.
int buscaBinaria(Compromisso** v, int n, long long chave, long* comparacoes) {
    int inicio = 0, fim = n - 1, achado = -1;
    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;
        (*comparacoes)++;
        long long atual = chaveData(*v[meio]);
        if (atual == chave) {
            achado = meio;
            fim = meio - 1;
        } else if (atual < chave) {
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
    }
    return achado;
}

// ---------- Menus ----------

// Mostra o menu principal.
void exibirMenu(int total) {
    cout << "\n===== AGENDA DE COMPROMISSOS (" << total << " cadastrados) =====\n"
         << " 1) Cadastrar        6) Desfazer exclusao (pilha)\n"
         << " 2) Listar por data  7) Lembretes (fila)\n"
         << " 3) Buscar por nome  8) Indice alfabetico (arvore)\n"
         << " 4) Editar           9) Comparar buscas por data\n"
         << " 5) Excluir         10) Gerar dados de teste\n"
         << " 0) Sair\n";
}

// Opcao 1: le os campos e cadastra.
void menuCadastrar(Agenda& a) {
    Compromisso novo;
    novo.nome = lerTexto("Nome");
    lerData(novo);
    lerHorario(novo);
    novo.local = lerTexto("Local");
    if (novo.nome.empty() || novo.local.empty()) return;
    cout << "  Cadastrado com ID " << cadastrar(a, novo) << ".\n";
}

// Opcao 2: ordena com Merge Sort e mostra (no maximo 50, para nao lotar o terminal).
void menuListar(const Agenda& a) {
    long comparacoes = 0;
    Compromisso** v = montarVetorOrdenado(a, &comparacoes);
    if (v == nullptr) {
        cout << "  Agenda vazia.\n";
        return;
    }
    cout << "Merge Sort: " << comparacoes << " comparacoes para " << a.total << " itens.\n";
    for (int i = 0; i < a.total && i < 50; i++) exibir(*v[i]);
    if (a.total > 50) cout << "  ... e mais " << (a.total - 50) << ".\n";
    delete[] v;
    v = nullptr;
}

// Opcao 3: busca sequencial por trecho do nome na lista.
void menuBuscar(const Agenda& a) {
    string trecho = minuscula(lerTexto("Trecho do nome"));
    int achados = 0;
    for (const NoLista* no = a.lista; no != nullptr; no = no->proximo) {
        if (minuscula(no->dados.nome).find(trecho) == string::npos) continue;
        exibir(no->dados);
        achados++;
    }
    if (achados == 0) cout << "  Nada encontrado.\n";
}

// Opcao 4: altera nome e local (Enter mantem) e, se quiser, data e horario.
void menuEditar(Agenda& a) {
    NoLista* no = buscarPorId(a, lerInteiro("ID", 1, 999999999));
    if (no == nullptr) {
        cout << "  ID nao encontrado.\n";
        return;
    }
    exibir(no->dados);
    string nome = lerTexto("Novo nome (Enter mantem)", false);
    string local = lerTexto("Novo local (Enter mantem)", false);
    if (!nome.empty()) no->dados.nome = nome;
    if (!local.empty()) no->dados.local = local;
    if (confirmar("Alterar data e horario")) {
        lerData(no->dados);
        lerHorario(no->dados);
    }
    cout << "  Atualizado.\n";
}

// Opcao 5: confirma, guarda uma copia na pilha (lixeira) e remove da lista.
void menuExcluir(Agenda& a) {
    NoLista* no = buscarPorId(a, lerInteiro("ID", 1, 999999999));
    if (no == nullptr) {
        cout << "  ID nao encontrado.\n";
        return;
    }
    exibir(no->dados);
    if (!confirmar("Excluir")) return;
    empilhar(a.lixeira, no->dados);
    removerPorId(a, no->dados.id);
    cout << "  Excluido (opcao 6 desfaz).\n";
}

// Opcao 6: pop na lixeira e devolve o ultimo excluido para a lista (mesmo ID).
void menuDesfazer(Agenda& a) {
    Compromisso recuperado;
    if (!desempilhar(a.lixeira, recuperado)) {
        cout << "  Nada para desfazer.\n";
        return;
    }
    inserirNoFim(a, recuperado);
    cout << "  Recuperado:\n";
    exibir(recuperado);
}

// Opcao 7: adicionar lembrete (enqueue), mostrar o proximo (dequeue) ou listar a fila.
void menuLembretes(Agenda& a) {
    cout << " 1) Adicionar  2) Mostrar proximo  3) Listar\n";
    int opcao = lerInteiro("Opcao", 1, 3);
    Compromisso c;
    if (opcao == 1) {
        NoLista* no = buscarPorId(a, lerInteiro("ID", 1, 999999999));
        if (no == nullptr) cout << "  ID nao encontrado.\n";
        else if (!enfileirar(a.lembretes, no->dados)) cout << "  Ja esta na fila.\n";
        else cout << "  Lembrete adicionado.\n";
    } else if (opcao == 2) {
        if (desenfileirar(a.lembretes, c)) exibir(c);
        else cout << "  Fila vazia.\n";
    } else {
        for (const NoFila* n = a.lembretes.inicio; n != nullptr; n = n->proximo) exibir(n->dados);
    }
}

// Opcao 8: mostra a ordem alfabetica (in-ordem) e busca um nome exato na arvore.
void menuArvore(Agenda& a) {
    if (a.total == 0) {
        cout << "  Agenda vazia.\n";
        return;
    }
    reconstruirArvore(a);
    int restante = 50;
    percorrerEmOrdem(a.raiz, &restante);
    string nome = lerTexto("Nome exato para buscar (Enter pula)", false);
    if (nome.empty()) return;
    long comparacoes = 0;
    int achados = buscarNaArvore(a.raiz, nome, &comparacoes);
    cout << "  " << achados << " encontrado(s) em " << comparacoes << " comparacoes.\n";
}

// Opcao 9: busca a mesma data de forma sequencial e binaria e compara as comparacoes.
void menuComparar(const Agenda& a) {
    Compromisso alvo;
    lerData(alvo);
    long ordenacao = 0, sequencial = 0, binaria = 0;
    Compromisso** v = montarVetorOrdenado(a, &ordenacao);
    if (v == nullptr) {
        cout << "  Agenda vazia.\n";
        return;
    }
    int pos1 = buscaSequencial(v, a.total, chaveData(alvo), &sequencial);
    int pos2 = buscaBinaria(v, a.total, chaveData(alvo), &binaria);
    cout << "  Itens: " << a.total << " | Merge Sort: " << ordenacao << " comparacoes (1 vez)\n"
         << "  Sequencial: " << sequencial << " comparacoes (indice " << pos1 << ")\n"
         << "  Binaria:    " << binaria << " comparacoes (indice " << pos2 << ")\n";
    delete[] v;
    v = nullptr;
}

// Opcao 10: cria compromissos de teste (valores calculados, sempre os mesmos).
void menuGerar(Agenda& a) {
    const char* tipos[5] = {"Prova", "Consulta", "Reuniao", "Evento", "Aula"};
    int quantidade = lerInteiro("Quantidade (1 a 5000)", 1, 5000);
    for (int i = 1; i <= quantidade; i++) {
        Compromisso c;
        c.nome = string(tipos[(i * 7) % 5]) + " " + to_string((i * 7919) % 10007);
        c.dia = 1 + (i * 11) % 28;
        c.mes = 1 + (i * 5) % 12;
        c.ano = 2026 + i % 2;
        c.hora = 7 + (i * 3) % 13;
        c.minuto = (i % 4) * 15;
        c.local = "Bloco " + string(1, (char)('A' + i % 4));
        cadastrar(a, c);
    }
    cout << "  " << quantidade << " compromissos criados. Dica: compare a data 26/12/2027.\n";
}

// Libera toda a memoria (cada new tem o seu delete).
void liberarAgenda(Agenda& a) {
    liberarLista(a);
    liberarArvore(a.raiz);
    Compromisso descarte;
    while (desempilhar(a.lixeira, descarte)) {}
    while (desenfileirar(a.lembretes, descarte)) {}
}

// Chama a funcao da opcao escolhida.
void executarOpcao(Agenda& a, int opcao) {
    switch (opcao) {
        case 1: menuCadastrar(a); break;
        case 2: menuListar(a); break;
        case 3: menuBuscar(a); break;
        case 4: menuEditar(a); break;
        case 5: menuExcluir(a); break;
        case 6: menuDesfazer(a); break;
        case 7: menuLembretes(a); break;
        case 8: menuArvore(a); break;
        case 9: menuComparar(a); break;
        case 10: menuGerar(a); break;
        default: break;
    }
}

// Laco principal: repete o menu ate digitar 0 (ou a entrada acabar).
int main() {
    Agenda agenda;
    int opcao = -1;
    while (opcao != 0) {
        exibirMenu(contarNos(agenda.lista));
        opcao = lerInteiro("Opcao", 0, 10);
        executarOpcao(agenda, opcao);
    }
    liberarAgenda(agenda);
    cout << "Ate logo!\n";
    return 0;
}
