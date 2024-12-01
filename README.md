Tema1A - Map-Reduce

# Main

    In main, se citesc datele de intrare, date ca argumente. Se afla numarul de threduri mapper, numarul de threaduri reducer, si
fisierul de input. Apoi din acest fisier de input se vor citi un numar de fisiere. Aceste fisiere sunt salvate intro coada de prioritati, prioritatea fiind dimensiunea fiecarui fisier. Apoi, initializam elementele folosite pentru sincronziare(2 mutexuri si 2 bariere). Initializam threadurile si le pornim. La final, le unim cu threadul main.

# Structura de Payload

    Structura de Payload este parametrul transims catre functia de thread. Aceasta contine toate detaliile dde care au nevoie
atat threadurile mapper cat si cele reducer. In primul rand cotine pointeri catre cele dou bariere si catre mutexuri. Contine numarul de threaduri din fiecrae, id-ul acestuia, dar si tipul de thread(0 pentru mapper, 1 pentru reducer). De asemenea, contine un pointer catre un vector de vectori care tine evidenta perechilor de cuvant fisier aparute in ficare fisier, un pointer catre coada de prioritati cu fisiere. In plus, pentru threadurile de tip Reducer, exista un pointer catre un map comun unde exista toate cuvintele din toate fisierele, dar si indexul fisierelor unde apar. In cele din urma, pentru fiecare thread avem un vector de(este vorba de Reducerr, chiar daca Mapperele contin acest camp, el nu este folosit) vector de vectori caruia ii sunt alocat un numar de litere, care contin cuvintele de la acea litera, care se gasesc in toate fisierele, dar si fisierele unde aceseta apar.

# Thread_Function - Mapper

    Partea de mapping se face cu ajutorul cozii de prioritati din main. Fiecare thread isi ia un fisier cat timp coada nu este
goala. Aici este prima zona ciritca, deoarece nu vrem ca mai multe threaduri sa ia in acelasi timp acelasi fisier. Asa ca aceasta zona va fi incadrata cu un mutex. Vom citi din fisierul proaspat extras toate cuvintele, conform constrangerilor. Introduce fiecare cuvant, impreuna cu id-ul fisierului in care acesta a fost gasit intr-un set, deoarece nu vrem sa punem de doua ori acelasi cuvant daca acesta apare de doua ori in in acelasi fisier. Dupa aceasta, setul este transformat in vector, si se adauga la vectorul de vectori de acest tip comun tuturor threadurilor. In cele din urma threadurile ajung la bariera si se opresc.

# Thread_Function - Reducer

    Partea de reduce incepe cu aceasi bariera unde se opresc si threaduirle mapper(aceasta bariera asteapta toate threadurile sa
ajunga la ea pentru a se asigura ca threadurile reducer nu incep executia pana nu au terminat de iterat prin fisiere toate threadurile de mapper). Alocam fiecarui thread un numar de vectori din vectorul de vectori comun si incepem sa populam mapul comun. Inseram fiecare cuvant intalnit, impreuna cu toate fisierele unde acesta apare. Ne vom intrebuinta de un map local pentru fiecare thread, pe care apoi il vom tranfera in mappul impartit de toate threadurile. Aici toate threadurile vor intalni o nou bariera care asteapta toate threadurile reducer, pentru a nu incepe urmatoarea etapa inainte ca mappul comun sa fie complet. In final se aloca un numar de litere pentru fiecare thread reducer. Fiecare thread va fi responsabil pentru crearea unui fisier reprezentativ pentru fiecare litra repartizata lui, si il populeaza cu toate cuvintele din mapul mare. De asemenea cuvintele, vor fi sortate intai dupa numarul de apariti, iar apoi daca numarul de aparitii este egal, lexicografic. In final, toate cuvintele sunt puse in ordinea creata in fisiere, impreuna cu indicele fisierelor in care apar.