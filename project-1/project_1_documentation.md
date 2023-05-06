Rinnakkaisuus COMP.CS.310 

Iivari Karrila K437292 

Akseli Ahonen 150313812 

Rasmus Ähtävä 150189389 

 

**Mitä rinnakkaisuuden ongelmia löysitte pohjakoodista?**

Qrtimer.cpp luokassa funktiossa init tulee kilpailutilanne, jos useampi säie käsittelee current_pixmap ja next_pixmap samanaikaisesti. Sama tilanne on myös funktiossa draw_board. Lisäksi tässä funktiossa on operaatio swap, joka voi toimia odottamattomalla tavalla, jos next_pixmap ja current_pixmap eivät ole synkronoituja sen kutsuhetkellä. 

Qrgraphics.cpp luokassa metodissa timerEvent ongelmana on, että GrTimer -säie ja world -säie eivät synkronoidu. World-säie voi päivittää pelitilaa samalla kun draw_board() käsittelee sitä GrTimer-säikeessä, mikä voi aiheuttaa kilpailutilanteen ja odottamattomia tuloksia. 

 

Main.cpp:  

Datakilpailu, pääsäikeen ja update-säikeen välillä ei ole synkronisaatiota. Molemmat säikeet voivat tämän vuoksi päätyä käyttämään yhtä aikaa world-statea. 

Resurssihukka, kun näiden kahden threadin välillä ei ole synkronisaatiota update-thread voi esim. päivittää maailmaa nopeammin kuin main kerkeää piirtää sitä. 

World.cpp -luokassa voi syntyä ongelmia next_generation funktion kanssa, jos useampi säie pääsee muokkaamaan pelitilaa samanaikaisesti. Tästä seuraa kilpailutilanne ja voidaan kokea odottamatonta toimintaa. 

 

**Miten em. ongelmat korjattiin palautuksessanne?**

Qrtimer luokassa lisättiin mutexit suojaamaan kriittisiä alueita sekä init funktioon että draw_board funktioon. 

Qrgraphics-luokassa luotiin synkronisaatiota update threadin ja timerEventin välille. 

Main luokassa on korjattu kilpailutilanne pääsäikeen ja update-säikeen välillä mutexilla ja sitten on suoritettu synkronisaatita draw_board funktion kanssa. 

World luokassa suojattiin next_generation funktiosta pelitilan muokkaaminen mutexilla, jotta vain yksi säie kerrallaan pystyy muokkaamaan pelitilaa. 

Luotiin ThreadPool –luokka käsittelemään säikeitä. Lisäksi valmista void next_generation() -metodia muokattiin siten, että se ottaa päivitettäväkseen vain rajallisen alueen pelikenttää. Lisättiin pelikentän pilkkomislogiikka mainiin jotta eri ThreadPool-säikeet suorittavat next_generation() -funktion omilla rajoilla. Threadpool odottaa kaikkien säikeiden valmistumista ja lopettaa itsensä. Tämän jälkeen uuden maailman iteraation pointteri vaihdetaan vanhaksi. 

Aluksi testattiin käynnistettävien säikeiden määrän määrittelyä koneen rautarajoitusten perusteella toimivan käskyn ( thread::hardware_concurrency ) perusteella. Tämä antoi käyttöömme 4 säiettä Tampereen virtuaalikoneella ja 16 omalla koneellamme. Koska tehtävän tarkoitus oli kuitenkin käynnistää varmuudella vähintään 10, päädyimme määrittelemään käynnistettävien säikeiden määrän manuaalisesti omalla num_threads –vakiollaan. 

 

**Selosta suunnitelmanne useamman säikeen käyttöön maailman päivityksessä ja miten toteutitte suunnitelmanne? (palautuksessa tulee selvästi näkyä mikä on ryhmän tekemää ja ja mikä on valmista pohjakoodia)**

Ryhmän tekemää: 

* Threadpool –luokka 

* Uusi world::next_generation() -metodi 

* World::update_concurrent() -metodi 

* mutex world_mutex, condition_variable world_cv, atomic<bool> world_updated 

* world::running muutettu atomiseksi

* Tickrate sekä maailmalle, että grafiikalle laskettu 200ms -> 20ms  

Suunnitelmana oli jakaa maailma chunkkeihin, joita jaetaan threadeille ajoon. Kaikkien threadien valmistuttua lopetetaan threadpool ja päivitetään iteraatio. Threadien synkronoituminen grafiikkapiirturin kanssa varmistetaan omalla lukituksellaan. Mutex world_mutex etc. pitää huolen, ettei update-thread(ja sen threadpool threadit) päivitä maailmaa samaan aikaan kuin main-thread piirtää sitä. 
