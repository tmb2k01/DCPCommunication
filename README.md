A példaprogram megmutatja hogyan lehetséges FMU modelleket futtatni Distributed Co-Simulation Protocol segítségével Ubuntu környezetben.


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