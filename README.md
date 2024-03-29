# IPK: projekt č. 1
Klient-server pro jednoduchý přenos souborů. Verze 0.3 (5.3.2018).

## Požadavky
### Překladač
Program lze přeložit v překladači podporující standard *C++11* (ISO/IEC 14882:2011). Doporučuje se překladač ***gcc* verze 4.8.4 a novější**. Pro jiné překladače a starší verze nebyl program testován.
### Knihovny potřebné k překladu
* iostream
* fstream
* unistd.h
* cstring
* sys/types.h
* sys/socket.h
* netinet/in.h
* netdb.h

## Překlad
Překlad lze provést programem *make*. Pro přeložení serveru i klienta použijte příkaz **```make all```** v adresáři s projektem. Pokud potřebujete provést překlad klienta nebo serveru jednotlivě, lze použít příkaz ```make ipk-client```, případně ```make server-ipk```.
Pokud není možné použít program \textit{make}, lze programy přeložit následujícími příkazy:
```
g++ -std=c++11 ipk-client.cpp -o ipk-client
g++ -std=c++11 ipk-server.cpp -o ipk-server
```

## Spuštění
### ipk-client
```
./ipk-client -h host -p port [-r|-w] soubor
  -h <host>
     (IP adresa nebo fully-qualified DNS name) identifikace serveru jakožto koncového bodu komunikace klienta
  -p <port>
    cílové číslo portu v intervalu 0-65535
  -r <soubor>
    klient stáhne soubor ze serveru
  -w <soubor>
    klient nahraje soubor na server
```
### ipk-server
```
./ipk-server -p <port>
  -p <port>
    číslo portu v intervalu <0;65535>, na kterém server naslouchá
```
Při použití čísla portu menší než 1024 mohou být pro spuštění vyžadována administrátorská oprávnění. Server se ukončuje signálem *SIGINT* (Ctrl+C).

## Návratové kódy
* 0 = program skončil v pořádku
* 1 = chyba při zpracování argumentu
* 2 = chyba síťového rozhraní (např: nelze vytvořit socket, přiřadit port, vytvořit spojení)
* 3 = chyba práce se soubory
* 4 = systémová chyba (selhal příkaz fork) - pouze u ipk-server
* 5 = chyba komunikace (klient zaslal neplatný požadavek)

## Známé chyby
Cílový i zdrojový adresář musí existovat. V případě, že neexistuje, končí program chybou, program adresáře nevytváří.

## Autoři
* **Vladan Kudláč** - autor projektu
