O exercicio 1 ficou realizado quase na sua totalidade, os testes relativos ao ponto 1.3 
revelam uma falha no multithreading que acreditamos ser relacionada com as funcoes de open e close,
no entanto apesar de horas a analisar nao conseguimos corrgir a tempo. Mais especificamente acreditamos 
que se deve a implementacao de trincos relativos a open_file_entry_t na funcao tfs_close e a sua chamada 
da funcao remove_from_open_file_table.

No que toca aos pontos 1.1, 1.2 e a concretizacao de todos os testes realizados, acreditamos ter atingido as metas propostas.