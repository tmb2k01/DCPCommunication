A példaprogram megmutatja hogyan lehetséges FMU modelleket futtatni Distributed Co-Simulation Protocol segítségével Ubuntu környezetben.

## Lokális környezetben tesztelés

A tesztelt környezet Debian 11, szükséges lesz C++ compiler, LibXML2 és XercesC library a program futtatásához, emellett CMake és Make a program fordításához.
Ezeket a következő parancsokkal lehet telepíteni:

```
sudo apt update
apt-get install -y cmake make g++ libxerces-c-dev libxml2 libxml2-dev libasio-dev unzip
```

A repository tárol egy **CMakeList.txt** fájlt, aminek a -DMODE paraméterével meghatározhatjuk hogy melyik komponenst szeretnénk telepíteni.

- cmake -DMODE=Master ..
- cmake -DMODE=Slave_One ..
- cmake -DMODE=Slave-Two ..

A buildelés megkönnyítésére van a **build.sh**, ami segít lebuildelni a példaprogramot, a Master-t és a 2 Slave-t.
Ennek a használata:
```
chmod +x build.sh
./build.sh
```
Ez létrehoz 3 könyvtárat, amikbe lefordítja a kódot. Ezután a könyvtárakban tudjuk futtatni az egyes komponenseket, az ```./fmusim``` paranccsal.

A **.vscode** mappában található a VSCode használatához szükséges beállítások. Mind a három komponensre meg van írva a Debuggoláshoz szükséges beállítás, így VSCode-ból könnyedén lehet debuggolni az egyes komponenseket.

A **lib** mappában található az FMU és a DCP futtatásához szükséges library-k, személyre szabva néhány helyen.

A **models** mappában találhatóak a futtatáshoz szükséges fájlok, modellek.
- Proxy.fmu -> A szimulációs modell, amit a Slave-k használnak
- MSDX-Slave-Description.xml -> leírófájl, ami a Slave-k tulajdonságait írja le
- data.cs -> SlaveOne-nak a bemenetét tartalmazó fájl.

A szimulációhoz először a két Slave komponenst kell elindítani, majd utána a Master-t, ami beregisztrálja a Slave komponenseket, és elindítja a szimulációt.

## Dockerizált környezetben tesztelés

A dockerizált környezetben az IP címeket át kell írni, ugyanis egy docker network segítségével fogunk létrehozni egy alhálózatot, és ebbe kapnak a dockerek IP címeket.

A dockerek létrehozásáhol a következő parancsot kell futtatni:
```
chmod +x build_dockers.sh
./build_dockers.sh
```
Ez a script létrehozza a docker imageket a repository-ban található Dockerfile-ok alapján.
  - Dockerfile.dcp_master
  - Dockerfile.slave_one
  - Dockerfile.slave_two
A docker imagek egy Debian 11-es dockert használnak, tartalmazzák a szükséges packageket a futtatáshoz, és a **build** mappába lefordítják az adott dockerhez tartozó komponenst.

Ezután a következő parancsokkal lehet létrehozni a containereket, és belépni a docker környezetébe. Dockerekbe belépve a szimulációt úgy tudjuk elindítani, ha a **build** mappába belépve kiadjuk a ```./fmusim``` parancsot. A példában megadott IP címek akkor működnek, ha a következő sorrendben indítjuk el a dockereket:
```
docker run -it --name master_container --network dcp-network master_image
docker run -it --name slave_one_container --network dcp-network slave_one_image
docker run -it --name slave_two_container --network dcp-network slave_two_image
```
Ezután a ```docker network inspect dcp-network``` paranccsal tudjuk ellenőrizni a containerek IP címeit.

Amint kiléptünk a containerekből, nem szükséges új containereket létrehozni, a következő parancsokkal el tudjuk indítani a meglévőket, és be tudunk lépni a containerekbe.
```
docker start master_container
docker start slave_one_container
docker start slave_two_container

docker exec -it master_container /bin/sh
docker exec -it slave_one_container /bin/sh
docker exec -it slave_two_container /bin/sh
```
