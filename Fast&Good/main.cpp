//GEREKLÝ KÜTÜPHANELER KAYNAK KODA EKLENÝR..
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>



// OYUNDA FARKLI PENCERELER GÖRÜNTÜLEMEK ÝÇÝN ENUM TANIMLANIR 4 FARKLI PENCERE ÝÇÝN TANIMLAMA YAPILIR..
enum class OyunDurumu { MENU, OYUN, AYARLAR, NASIL_OYNANIR };






// BÝR STRUCT TANIMLANIR, HER BÝR ÜRÜN ÝÇÝN ÝSÝM, RESÝM, SPRITE, ORÝJÝNAL KONUM VE SEÇÝLÝ MÝ BÝLGÝSÝ TUTULUR
struct Urun {
    std::string isim;                   // Urunun ismi ("Kola", "Pizza" gibi)
    sf::Texture texture;               // Urunun resmi (texture)
    sf::Sprite sprite;                 // Texture'un ekranda cizilecek hali
    sf::Vector2f orijinalKonum;        // Urunun baslangic pozisyonu
    bool secildiMi = false;            // Bu urun secili mi/surukleniyor mu bilgisi
};

// MAÝN FONKSÝYON DIÞINDA URUNLERÝ OLUÞTURMAK ÝÇÝN FONKSÝYON TANIMLANIR, BU FONKSÝYON, ÜRÜN ÝSMÝ, RESÝM YOLU, X VE Y KOORDÝNATLARI, VE ÖLÇEK X VE Y DEÐERLERÝNÝ ALIR.
Urun* urunOlustur(const std::string& isim, const std::string& texturePath, float x, float y, float scaleX, float scaleY) {
    Urun* urun = new Urun;
    if (!urun->texture.loadFromFile(texturePath)) {
        std::cerr << isim << " gorseli yuklenemedi!\n";
        delete urun;
        return nullptr;
    }
    urun->isim = isim;
    urun->sprite.setTexture(urun->texture);
    urun->sprite.setPosition(x, y);
    urun->sprite.setScale(scaleX, scaleY);
    urun->orijinalKonum = { x, y };
    return urun;
}

// OYUN EKRANINDA BELLÝ BÝR SÜRE KALACAK BÝLDÝRÝM GÖSTERMEK ÝÇÝN FONKSÝYON TANIMLANIR, BU FONKSÝYON, METÝN, MESAJ VE ZAMANLAYICI ALIR.  
void guncelleBildirim(sf::Text& text, const std::string& mesaj, sf::Clock& saat) {
    text.setString(mesaj);
    saat.restart(); // Bildirimi gösterme süresini baþlat
}

// OYUNCU KAZANÇLARINI DOSYAYA YAZMAK ÝÇÝN FONKSÝYON TANIMLANIR, BU FONKSÝYON, OYUNCU ÝSMÝ VE KASA BAKÝYESÝ ALIR. EÐER OYUNCU ÝSMÝ DOSYADA YOKSA, YENÝ VERÝ EKLENÝR.
void dosyayaYazdir(const std::string& oyuncuIsmi, float kasaBakiyesi) {
    std::ifstream dosyaOku("kazanc_listesi.txt"); // Dosyayý okuma modunda aç
    bool isimVarMi = false;

    if (dosyaOku.is_open()) {
        std::string line;
        while (getline(dosyaOku, line)) {
            if (line.find(oyuncuIsmi) != std::string::npos) { // Eðer isim dosyada varsa
                isimVarMi = true;
                break;
            }
        }
        dosyaOku.close();
    }

    // Eðer oyuncu ismi dosyada yoksa, yeni veriyi ekle
    if (!isimVarMi) {
        std::ofstream dosya("kazanc_listesi.txt", std::ios::app); // Dosyayý ekleme modunda aç
        if (dosya.is_open()) {
            dosya << oyuncuIsmi << " : " << kasaBakiyesi << " TL Kazanclar\n";
            dosya.close();
        }
        else {
            std::cerr << "Dosya acilamadi!\n";
        }
    }
}





int main() {

    

    sf::RenderWindow window(sf::VideoMode(800, 600), "FAST&GOOD"); // OYUN PENCERESÝ OLUÞTURULUR
    OyunDurumu oyunDurumu = OyunDurumu::MENU;                      // OYUN DURUMU BAÞLANGIÇTA MENÜ OLARAK AYARLANIR

    std::string oyuncuIsmi = "";                                   // OYUNCU ÝSMÝ BAÞLANGIÇTA BOÞTUR, OYUNCU BUNU GÝRÝNCE DOLDURULACAKTIR


    bool isimAlindi = false;

    bool siparisHazir = false;

    bool siparisDogruMu = false;

    bool siparisSonrasiBekleme = false;

    bool yeniSiparisBekleniyor = false;

    bool mouseBasili = false;


    float rastgeleBeklemeSuresi = 0.0f;

    float kasaBakiyesi = 500.0f;

    float beklemeSuresi = 0.0f; // Bekleme suresi, saniye cinsinden



    std::vector<std::string> verilenUrunler;

    std::vector<std::string> alinanUrunler;



    sf::Clock beklemeSaati;

    sf::Clock gecenSure;

    sf::Clock bildirimSaati;





    sf::SoundBuffer coinBuffer;
    if (!coinBuffer.loadFromFile("assets/sounds/coin.wav")) {
        std::cerr << "Coin sesi yuklenemedi!\n";
    }
    sf::Sound coinSound(coinBuffer);

    sf::SoundBuffer siparisBuffer;
    siparisBuffer.loadFromFile("assets/sounds/zil.wav");
    sf::Sound siparisSesi(siparisBuffer);










    // === MENÜ ; OYUN MENÜSÜNDEKÝ ARKAPLAN RESMÝNÝ YÜKLER ===
    sf::Texture menuTexture;
    if (!menuTexture.loadFromFile("C:/Users/seraf/source/repos/Fast&Good/Fast&Good/assets/images/arkaplan/menu.png")) { // === BURADAKÝ ÝF BLOÐU GÖRSELÝN YÜKLENMESÝ ÝÇÝN KULLANILIR,  GÖRSEL YÜKLENEMEZSE  PROGRAMI KAPATIR..(bu her görsel yükleme alanýnda kullanýlýr.)
        std::cerr << "Menu resmi yuklenemedi!\n";
        return 1;
    }

    sf::Sprite menuSprite(menuTexture);
    menuSprite.setScale(
        (float)window.getSize().x / menuTexture.getSize().x,  // === BU SATIRLAR SPRÝTE(GÖRSEL)'ÝN OYUN EKRANINI TAMAMEN KAPLAMASI ÝÇÝN YAZILIR..
        (float)window.getSize().y / menuTexture.getSize().y   // === BU SATIRLAR SPRÝTE(GÖRSEL)'ÝN OYUN EKRANINI TAMAMEN KAPLAMASI ÝÇÝN YAZILIR..
    );


    // === MENÜ-yazi ; OYUN MENÜSÜNDEKÝ YAZÝYÝ YÜKLER ===
    sf::Texture menuYaziTexture;
    if (!menuYaziTexture.loadFromFile("assets/images/arkaplan/fastgood.png")) {
        std::cerr << "Menu yazisi resmi yuklenemedi!\n";
        return 1;
    }

    sf::Sprite menuYaziSprite(menuYaziTexture);
    menuYaziSprite.setPosition(165, 10); // Yazýnýn pozisyonu
    menuYaziSprite.setScale(0.45f, 0.3f); // Yazýnýn boyutu 


    // === MENÜ-oyna ; OYUN MENÜSÜNDEKÝ OYNA BUTONUNU YÜKLER ===
    sf::Texture menuOynaTexture;
    if (!menuOynaTexture.loadFromFile("assets/images/arkaplan/oyna.png")) {
        std::cerr << "Oyna butonu resmi yuklenemedi!\n";
        return 1;
    }

    sf::Sprite menuOynaSprite(menuOynaTexture);
    menuOynaSprite.setPosition(175, 250); // butonun pozisyonu
    menuOynaSprite.setScale(0.18f, 0.18f); // butonun boyutu 


    // === MENÜ-çýkýþ ; OYUN MENÜSÜNDEKÝ ÇIKIÞ BUTONUNU YÜKLER ===
    sf::Texture menuCikisTexture;
    if (!menuCikisTexture.loadFromFile("assets/images/arkaplan/cikis.png")) {
        std::cerr << "cikis butonu resmi yuklenemedi!\n";
        return 1;
    }

    sf::Sprite menuCikisSprite(menuCikisTexture);
    menuCikisSprite.setPosition(425, 400); //  butonun pozisyonu
    menuCikisSprite.setScale(0.18f, 0.18f); // butonun boyutu 


    // === MENÜ-ayarlar; OYUN MENÜSÜNDEKÝ AYARLAR BUTONUNU YÜKLER ===     === AYARLAR KISMI ÝÞLEVSÝZ KALACAKTIR ===
    sf::Texture menuAyarlarTexture;
    if (!menuAyarlarTexture.loadFromFile("assets/images/arkaplan/ayarlar.png")) {
        std::cerr << "Ayarlar butonu resmi yuklenemedi!\n";
        return 1;
    }

    sf::Sprite menuAyarlarSprite(menuAyarlarTexture);
    menuAyarlarSprite.setPosition(175, 400); // butonun pozisyonu
    menuAyarlarSprite.setScale(0.18f, 0.18f); // butonun boyutu 


    // === MENÜ-nasilOynanir ; OYUN MENÜSÜNDEKÝ  NASIL OYNANIR BUTONUNU YÜKLER ===
    sf::Texture menuNasilOynanirTexture;
    if (!menuNasilOynanirTexture.loadFromFile("assets/images/arkaplan/nasiloynanir.png")) {
        std::cerr << "Nasil oynanir butonu resmi yuklenemedi!\n";
        return 1;
    }

    sf::Sprite menuNasilOynanirSprite(menuNasilOynanirTexture);
    menuNasilOynanirSprite.setPosition(425, 250); //  butonun pozisyonu
    menuNasilOynanirSprite.setScale(0.18f, 0.18f); //  butonun boyutu 








    // === OYUNUN HAKKINDA BÖLÜMÜNDE KULLANILACAK SPRÝTE(GÖRSEL) ÝÇERÝÐÝ YÜKLENÝR ===
    sf::Texture hakkindaTexture;
    if (!hakkindaTexture.loadFromFile("assets/images/arkaplan/hakkinda.png")) {
        std::cerr << "Hakkinda butonu resmi yuklenemedi!\n";
        return 1;
    }

    sf::Sprite hakkindaSprite(hakkindaTexture);
    hakkindaSprite.setScale(
        (float)window.getSize().x / hakkindaTexture.getSize().x,
        (float)window.getSize().y / hakkindaTexture.getSize().y
    );








    // === OYUNA GÝRÝÞ YAPILDIÐINDA YÜKLENECEK OLAN ARKA PLAN GÖRÜNTÜSÜ YÜKLENÝR ===
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("assets/images/arkaplan/arkaplan.png")) {
        std::cerr << "Arka plan resmi yuklenemedi!\n";
        return 1;
    }
    sf::Sprite background(backgroundTexture);
    background.setScale(
        (float)window.getSize().x / backgroundTexture.getSize().x,
        (float)window.getSize().y / backgroundTexture.getSize().y
    );


    // === OYUNA GÝRÝÞ YAPILDIÐINDA YÜKLENECEK OLAN BÜTÜN GÖRSELLERÝN GÖRÜNTÜSÜ AÞAÐIDA SIRAYLA YÜKLENÝR ===    
    sf::Texture onaylaButonTexture;
    if (!onaylaButonTexture.loadFromFile("assets/images/arkaplan/buton.png")) {
        std::cerr << "Buton resmi yuklenemedi!\n";
        return 1;
    }
    sf::Sprite onaylaButonSprite(onaylaButonTexture);
    onaylaButonSprite.setPosition(0, 320);
    onaylaButonSprite.setScale(0.1f, 0.1f); // Buton boyutunu ayarla 


    // === KONUSMA BALONU ===
    sf::Texture balonTexture;
    if (!balonTexture.loadFromFile("assets/images/arkaplan/konusma.png")) {
        std::cerr << "Balon resmi yuklenemedi!\n";
        return 1;
    }
    sf::Sprite balon(balonTexture);
    balon.setPosition(470, -20);
    balon.setScale(0.28f, 0.28f);











    Urun* seciliUrun = nullptr; // Seçili ürün, sürüklenen ürün için tutulur    


    //urun kopyasý için.. === ÜRÜN SÜRÜKLENDÝÐÝNDE ÜRÜNÜN KOPYASI OLARAK TUTULUR ===
    sf::Sprite* suruklenenUrun = nullptr;
    sf::Vector2f fark;


    // === ÜRÜN FÝYATLARINI TUTMAK ÝÇÝN MAP OLUÞTURULUR ===
    std::map<std::string, float> urunFiyati = {
    {"Sucuklu Pizza", 150.0f},
    {"Tavuk Burger", 120.0f},
    {"Et Burger", 180.0f},
    {"Margarita Pizza", 100.0f},
    {"Patates", 50.0f},
    {"Kola", 40.0f},
    {"Gazoz", 40.0f}
    };

    // === ÜRÜN MAALÝYETLERÝNÝ TUTMAK ÝÇÝN MAP OLUÞTURULUR === SABÝT DEÐÝÞKEN OLARAK MAALÝYETORANI 0.8f TANIMLANIR, BU DEÐER ÜRÜN FÝYATLARININ %80'Ý OLARAK KULLANILIR. BU SAYEDE ÜRÜN MAALÝYETLERÝ HESAPLANIR. 
    const float maaliyetOrani = 0.8f;

    std::map<std::string, float> urunMaaliyetleri;

    // === DÖNGÜ YARDIMIYLA ÜRÜN MAALÝYETLERÝNÝ HESAPLAR ===
    for (const auto& pair : urunFiyati) {
        urunMaaliyetleri[pair.first] = pair.second * 0.8f;
    }


    // === MUSTERI HAVUZUNU OLUÞTURMA ===    === MÜÞTERÝLER ÝÇÝN DOSYA ADLARI VE SÝPARÝÞ METÝNLERÝ TANIMLANIR === === FOR DÖNGÜSÜ ÝLE MÜÞTERÝ DOSYALARI YOLU ALINIR ===
    std::vector<std::string> musteriDosyalari;
    for (int i = 1; i <= 10; i++) {
        musteriDosyalari.push_back("customer" + std::to_string(i) + ".png");
    }
    std::vector<std::string> siparisMetinleri = {
    "     Iyi günler bir\nadet Sucuklu Pizza \n           istiyorum!",
    "         Selamlar \n Tavuk Burger ve Kola\nalabilir miyim lutfen?",
    "   Sucuklu Pizza \n   yanina Kola!",
    "        Et Burger\n  istiyorum kola\nolmasin diyetteyim!",
    "Merhaba Et Burger\n      ve Kola\n  alabilir miyim?",
    "       Bir adet\nTavuk Burger ve\n yanina Gazoz\n istiyorum, lutfen!",
    " Merhabalar cok acim\n Sucuklu Pizza yaninda\n Gazoz lutfen!",
    "  Selam, Patates\n  var mi? Evet\n  sadece patates\n    istiyorum!",
    "    Tavuk Burger\n   Patates ve Kola\n          lutfen!!",
    "  Kolay gelsin,\n  sey ben bir tane \n  Kola alicaktim!",
    "      Merhaba\n Margarita Pizza \n  alicaktim ben."
    };

    // === URUNLERI OLUSTUR ===  === BAÞTA YAZILAN urunOlustur FONKSÝYONU KULLANILARAK ÜRÜNLER OLUÞTURULUR === === DOSYA YOLU POZÝSYONLARI VE ÖLÇEKLERÝ BELÝRLENÝR ===  
    std::vector<Urun*> urunler;
    urunler.push_back(urunOlustur("Kola", "assets/images/food/coke.png", 10, 415, 0.5f, 0.5f));
    urunler.push_back(urunOlustur("Gazoz", "assets/images/food/soda.png", 57, 420, 0.17f, 0.165f));
    urunler.push_back(urunOlustur("Tavuk Burger", "assets/images/food/chickenburger.png", 100, 435, 0.18f, 0.18f));
    urunler.push_back(urunOlustur("Et Burger", "assets/images/food/meatburger.png", 210, 420, 0.45f, 0.45f));
    urunler.push_back(urunOlustur("Margarita Pizza", "assets/images/food/margaritapizza.png", 330, 420, 0.5f, 0.5f));
    urunler.push_back(urunOlustur("Patates", "assets/images/food/patato.png", 570, 450, 0.35f, 0.35f));
    urunler.push_back(urunOlustur("Sucuklu Pizza", "assets/images/food/sucukpizza.png", 450, 420, 0.28f, 0.28f));


    // === RASTGELE MUSTERI VE SIPARIS METINI SECME ===  === RASTGELE SEÇÝM ÝÇÝN SRAND() FONKSÝYONU KULLANILIR ===  === CÜMLENÝN ÝÇÝNDE GEÇEN ÜRÜNLERÝ BELÝRLEMEK ÝÇÝN FIND() FONKSÝYONU KULLANILIR ===

    srand(static_cast<unsigned>(time(0)));
    std::string secilenMusteri = musteriDosyalari[rand() % musteriDosyalari.size()];
    std::string secilenSiparis = siparisMetinleri[rand() % siparisMetinleri.size()];

    std::vector<std::string> siparisUrunleri;

    if (secilenSiparis.find("Kola") != std::string::npos)
        siparisUrunleri.push_back("Kola");
    if (secilenSiparis.find("Gazoz") != std::string::npos)
        siparisUrunleri.push_back("Gazoz");
    if (secilenSiparis.find("Tavuk Burger") != std::string::npos)
        siparisUrunleri.push_back("Tavuk Burger");
    if (secilenSiparis.find("Et Burger") != std::string::npos)
        siparisUrunleri.push_back("Et Burger");
    if (secilenSiparis.find("Margarita Pizza") != std::string::npos)
        siparisUrunleri.push_back("Margarita Pizza");
    if (secilenSiparis.find("Patates") != std::string::npos)
        siparisUrunleri.push_back("Patates");
    if (secilenSiparis.find("Sucuklu Pizza") != std::string::npos)
        siparisUrunleri.push_back("Sucuklu Pizza");



    // === MUSTERI ===
    sf::Texture musteriTexture;
    if (!musteriTexture.loadFromFile("assets/images/halfcustomer/" + secilenMusteri)) {
        std::cerr << "Musteri resmi yuklenemedi!\n";
        return 1;
    }
    sf::Sprite musteri(musteriTexture);
    musteri.setPosition(150, 55);
    musteri.setScale(0.5f, 0.5f);


    // === DIGER NESNELER === === KASA, POSET VE BILDIRIM PANELI GÖRSELLERÝ YÜKLENÝR ===
    sf::Texture kasaTexture;
    kasaTexture.loadFromFile("assets/images/food/kasa.png");
    sf::Sprite kasa(kasaTexture);
    kasa.setPosition(635, 360);
    kasa.setScale(0.17f, 0.17f);

    sf::Texture posetTexture;
    posetTexture.loadFromFile("assets/images/arkaplan/poset.png");
    sf::Sprite poset(posetTexture);
    poset.setPosition(500, 280);
    poset.setScale(0.15f, 0.15f);

    sf::Texture bildirimPaneliTexture;
    bildirimPaneliTexture.loadFromFile("assets/images/arkaplan/bildirim.png");
    sf::Sprite bildirimPaneli(bildirimPaneliTexture);
    bildirimPaneli.setPosition(185, -100); // Bildirim panelinin pozisyonu
    bildirimPaneli.setScale(0.4f, 0.2f); // Ýsteðe baðlý olarak boyutlandýrma



    sf::Texture paraButonTexture;
    paraButonTexture.loadFromFile("assets/images/arkaplan/kasabutton.png");
    sf::Sprite paraButonSprite(paraButonTexture);
    paraButonSprite.setPosition(620, 500); // Butonun pozisyonu
    paraButonSprite.setScale(0.185f, 0.1f); // Kasanýn boyutu  



    // === YAZI VE SES === === YAZI ÝÇÝN FONT YÜKLENÝR VE YAZI ÖZELLÝKLERÝ AYARLANIR === === BÝLDÝRÝM YAZISI VE BAKÝYE YAZISI OLUÞTURULUR ===
    sf::Font font;
    font.loadFromFile("assets/fonts/arial.ttf");


    sf::Text oyuncuIsimYazisi;
    oyuncuIsimYazisi.setFont(font);
    oyuncuIsimYazisi.setCharacterSize(24);
    oyuncuIsimYazisi.setFillColor(sf::Color::White);
    oyuncuIsimYazisi.setString("Oyuncu adinizi girin: ");
    oyuncuIsimYazisi.setPosition(200, 200); // Yazýnýn pozisyonu






    // === BÝLDÝRÝM METNÝ ===  === BÝLDÝRÝM PANELÝNDE GÖSTERÝLECEK METÝN ===
    std::string bildirimMetni = "";

    sf::Text bildirimYazisi;

    bildirimYazisi.setFont(font);
    bildirimYazisi.setCharacterSize(24);
    bildirimYazisi.setFillColor(sf::Color::Black);
    bildirimYazisi.setStyle(sf::Text::Bold);
    bildirimYazisi.setPosition(200, 20);  // Yukarýdan bildirimin pozisyonu






    sf::Text bakiyeYazisi;

    bakiyeYazisi.setFont(font);
    bakiyeYazisi.setCharacterSize(20);
    bakiyeYazisi.setFillColor(sf::Color::White);
    bakiyeYazisi.setPosition(
        kasa.getPosition().x + 10,                                     // === BAKÝYE YAZISI KASA GÖRSELÝNÝN ALTINDA OLACAK ÞEKÝLDE AYARLANMIÞTIR ===
        kasa.getPosition().y + kasa.getGlobalBounds().height + 5       // === BAKÝYE YAZISI KASA GÖRSELÝNÝN ALTINDA OLACAK ÞEKÝLDE AYARLANMIÞTIR ===
    );
    bakiyeYazisi.setString("Kasa: " + std::to_string(kasaBakiyesi).substr(0, 6) + " TL");  // === KASA BAKÝYESÝ YAZISI ===   === SUBSTR() FONKSÝYONU KASA BAKÝYESÝNÝN 6 HANELÝ OLARAK GÖSTERÝLMESÝNÝ SAÐLAR ===

    sf::Text siparisYazisi(secilenSiparis, font, 18);
    siparisYazisi.setFillColor(sf::Color::Black);
    siparisYazisi.setPosition(540, 75);



    // === OYUN DONGUSU === ===== OYUN DÖNGÜSÜ BAÞLAR ===
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));  // === FARE POZÝSYONUNU mapPixelToCoords ÝFADESÝYLE DÜNYA KOORDÝNATLARINA DÖNÜÞTÜRÜR ===
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));



            if (!isimAlindi) {                                                     // Eðer isim henüz alýnmamýþsa BLOK ÇALIÞIR
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 13) {                                // Eðer Enter tuþuna basýldýðýnda
                        isimAlindi = true;                                         // Ýsmi alýndý olarak iþaretle
                        oyuncuIsmi = oyuncuIsimYazisi.getString();                 // Girilen ismi al
                        std::cout << "Oyuncu Ýsmi: " << oyuncuIsmi << std::endl;   // Giriþ kontrolü
                    }
                    else if (event.text.unicode < 128) {                            // Eðer basýlan tuþ ASCII karakteriyse
                        oyuncuIsmi += static_cast<char>(event.text.unicode);        // Girilen karakteri isme ekle
                        oyuncuIsimYazisi.setString("Oyuncu adinizi girin: " + oyuncuIsmi);  // Ekranda göster
                    }
                }
            }
            if (isimAlindi) {
                oyunDurumu = OyunDurumu::OYUN;  // Oyun ismi alýndýktan sonra baþlayacak
            }





            // ===SWÝTCH CASE YAPISI ÝLE oyunDurumu KONTROL EDÝLEREK 4 FARKLI CASE OLUÞTURULUR MENU , OYUN , AYARLAR , NASIL_OYNANIR ===
            switch (oyunDurumu) {
            case OyunDurumu::MENU:


                if (isimAlindi) {
                    oyunDurumu = OyunDurumu::OYUN;  // Oyun ismi alýndýktan sonra baþlayacak
                }

                if (oyunDurumu == OyunDurumu::MENU && !isimAlindi) {
                    // Ýsim alma iþlemi
                    window.draw(oyuncuIsimYazisi);
                    if (isimAlindi) {
                        oyunDurumu = OyunDurumu::OYUN; // Oyuna baþla
                    }
                }

                // Fare pozisyonunu al

                // Hover efekti   === BURADA FARE POZÝSYONU ÝLE BUTONLARIN GLOBAL BOUNDS'INI KONTROL EDEREK HOVER EFEKTÝ UYGULANIR === === YANÝ FARE BUTON GÖRSELLERÝNÝN ÜSTÜNE GELÝRSE BUTONLAR PARLAKLIK KAZANIR ===
                if (menuOynaSprite.getGlobalBounds().contains(mousePos)) {
                    menuOynaSprite.setColor(sf::Color(255, 255, 255, 180)); // Hafif þeffaflýk efekti
                }
                else {
                    menuOynaSprite.setColor(sf::Color::White); // Orijinal rengini geri getir
                }

                if (menuCikisSprite.getGlobalBounds().contains(mousePos)) {
                    menuCikisSprite.setColor(sf::Color(255, 255, 255, 180)); // Hafif þeffaflýk efekti
                }
                else {
                    menuCikisSprite.setColor(sf::Color::White); // Orijinal rengini geri getir
                }

                if (menuAyarlarSprite.getGlobalBounds().contains(mousePos)) {
                    menuAyarlarSprite.setColor(sf::Color(255, 255, 255, 180)); // Hafif þeffaflýk efekti
                }
                else {
                    menuAyarlarSprite.setColor(sf::Color::White); // Orijinal rengini geri getir
                }

                if (menuNasilOynanirSprite.getGlobalBounds().contains(mousePos)) {
                    menuNasilOynanirSprite.setColor(sf::Color(255, 255, 255, 180)); // Hafif þeffaflýk efekti
                }
                else {
                    menuNasilOynanirSprite.setColor(sf::Color::White); // Orijinal rengini geri getir
                }

                // Týklama iþlemi
                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                    // === FARE OYNA BUTONUN ÜZERÝNDEYKEN SOL CLÝCK YAPARSA CASE DURMUNUN OYUN A DÖNMESÝ KONTROLÜ SAÐLANIR ===
                    if (menuOynaSprite.getGlobalBounds().contains(mousePos)) {             // === FARE OYNA BUTONUN ÜZERÝNDEYKEN SOL CLÝCK YAPARSA CASE DURMUNUN OYUN A DÖNMESÝ KONTROLÜ SAÐLANIR ===
                        oyunDurumu = OyunDurumu::OYUN;  // Oyun moduna geçiþ
                        siparisSesi.play();  // Oyun baþlama sesi
                    }

                    // === FARE ÇIKIÞ BUTONUN ÜZERÝNDEYKEN SOL CLÝCK YAPARSA CASE DURMUNUN ÇIKIÞ A DÖNMESÝ KONTROLÜ SAÐLANIR ===
                    if (menuCikisSprite.getGlobalBounds().contains(mousePos)) {
                        window.close();  // Uygulamayý kapatma
                    }

                    // === FARE OYNA BUTONUN ÜZERÝNDEYKEN SOL CLÝCK YAPARSA CASE DURMUNUN AYARLAR A DÖNMESÝ KONTROLÜ SAÐLANIR === === BÝLGÝ EKSÝKLÝÐÝ VE KARMAÞILAÞMA DURUMUNDAN BU BUTONUN ÝÞLEVÝ YOKTUR ===
                    if (menuAyarlarSprite.getGlobalBounds().contains(mousePos)) {
                        std::cout << "Ayarlar butonuna týklandý!\n";
                        // Ayarlar ekranýna geçiþ iþlemleri burada yapýlabilir
                    }

                    // === FARE OYNA BUTONUN ÜZERÝNDEYKEN SOL CLÝCK YAPARSA CASE DURMUNUN NASIL_OYNANIR A DÖNMESÝ KONTROLÜ SAÐLANIR ===
                    if (menuNasilOynanirSprite.getGlobalBounds().contains(mousePos)) {
                        std::cout << "Nasýl Oynanýr butonuna týklandý!\n";
                        oyunDurumu = OyunDurumu::NASIL_OYNANIR;

                    }
                }
                break;
            case OyunDurumu::OYUN:                   // === OYUN DURUMU CASE Ý AÇILIR BÜTÜN SÜRÜKLEME FONKSÝYONEL ÝÞLEMLER BU CASE DE GERÇEKLEÞECEKTÝR ===
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {   // OYUNCU OYUN SEKMESÝ AÇIKKEN KLAVYEDEN ESC TUÞUNA BASARSA MENÜ EKRANINA DÖNME KONTROLÜ
                    oyunDurumu = OyunDurumu::MENU;  // Menü moduna geçiþ
                }
                // Sürükleme býrakma
                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                    mouseBasili = false;
                    if (suruklenenUrun != nullptr) {
                        if (suruklenenUrun->getGlobalBounds().intersects(poset.getGlobalBounds())) {
                            if (seciliUrun != nullptr) {
                                verilenUrunler.push_back(seciliUrun->isim);
                                std::cout << seciliUrun->isim << " posete birakildi!\n";
                                guncelleBildirim(bildirimYazisi, seciliUrun->isim + " posete birakildi!", bildirimSaati);
                                bildirimPaneli.setScale(0.4f, 0.2f);  // Normal hale geri getir


                                alinanUrunler.push_back(seciliUrun->isim);

                                kasaBakiyesi -= urunMaaliyetleri[seciliUrun->isim];
                                bakiyeYazisi.setString("Kasa: " + std::to_string(kasaBakiyesi).substr(0, 6) + " TL");

                                if (alinanUrunler.size() == siparisUrunleri.size()) {
                                    siparisHazir = true;
                                }


                                siparisDogruMu = true;
                                for (const auto& sipUrun : siparisUrunleri) {
                                    if (std::find(alinanUrunler.begin(), alinanUrunler.end(), sipUrun) == alinanUrunler.end()) {
                                        siparisDogruMu = false;

                                        if (alinanUrunler.size() == siparisUrunleri.size()) {
                                            siparisDogruMu = std::is_permutation(alinanUrunler.begin(), alinanUrunler.end(), siparisUrunleri.begin());
                                        }
                                        else {
                                            siparisDogruMu = false;
                                        }


                                    }
                                }


                            }
                        }
                        else {
                            delete suruklenenUrun;        // === SÜRÜKLENEN ÜRÜNÜ SÝL ===
                        }

                        suruklenenUrun = nullptr;  // === SÜRÜKLENEN ÜRÜNÜ SIFIRLA ===
                        seciliUrun = nullptr;       // === SEÇÝLÝ ÜRÜNÜ SIFIRLA ===
                    }

                    else if (onaylaButonSprite.getGlobalBounds().contains(worldPos)) {        // === ONAYLA BUTONUNA TIKLANDIYSA ===


                        if (siparisHazir && !siparisSonrasiBekleme) {      // === SÝPARÝÞ HAZIRSA VE SÝPARÝÞ SONRASI BEKLEME DURUMU YOKSA ===
                            // Ürünler doðru mu?
                            if (alinanUrunler.size() == siparisUrunleri.size()) {
                                siparisDogruMu = std::is_permutation(
                                    alinanUrunler.begin(), alinanUrunler.end(), siparisUrunleri.begin()   // === ALINAN ÜRÜNLERÝN SÝPARÝÞ ÜRÜNLERÝ ÝLE KARÞILAÞTIRILMASI ===
                                );
                            }
                            else {
                                siparisDogruMu = false;
                            }

                            if (siparisDogruMu) {
                                std::cout << "Tebrikler! Siparis dogru hazirlandi.\n";



                                float toplamGelir = 0.0f;
                                for (const auto& urun : siparisUrunleri)
                                    toplamGelir += urunFiyati[urun];

                                kasaBakiyesi += toplamGelir;  // === KASA BAKÝYESÝNE TOPLAM GELÝR EKLENÝR ===
                                coinSound.play(); // Sipariþ doðruysa ses çal  // === SÝPARÝÞ DOÐRUYSA  SES ÇALINIR ===
                                bakiyeYazisi.setString("Kasa: " + std::to_string(kasaBakiyesi).substr(0, 6) + " TL");
                                std::cout << "Kasaya " << toplamGelir << " TL eklendi!\n";
                                guncelleBildirim(bildirimYazisi, "Siparis doðru! Kasaya " + std::to_string(toplamGelir).substr(0, 6) + " TL eklendi!", bildirimSaati);
                                bildirimPaneli.setScale(0.5f, 0.2f);  // Panelin eni %50 büyütüldü


                                // Müþteri ve sipariþi sahneden kaldýr
                                siparisYazisi.setString("");
                                musteri.setColor(sf::Color::Transparent);
                                balon.setColor(sf::Color::Transparent);
                                poset.setColor(sf::Color::Transparent);

                                // Yeni sipariþ için hazýrlýk
                                gecenSure.restart();
                                rastgeleBeklemeSuresi = 4.0f + rand() % 6; // 4-9 saniye
                                siparisSonrasiBekleme = true;
                            }
                            else {        // === SÝPARÝÞ YANLIÞSA ===
                                std::cout << "Siparis dogru degil, kontrol edin!\n";
                                guncelleBildirim(bildirimYazisi, "Siparis yanlis! Kontrol edin.", bildirimSaati);
                                bildirimPaneli.setScale(0.4f, 0.2f);  // Normal hale geri getir
                                alinanUrunler.clear();
                                verilenUrunler.clear();


                            }
                        }


                    }
                }


                if (mouseBasili && suruklenenUrun != nullptr) {   // === SÜRÜKLEME DURUMUNDA SÜRÜKLENEN ÜRÜNÜN POZÝSYONUNU GÜNCELLE ===
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    suruklenenUrun->setPosition(mousePos - fark);
                }

                // Yeni sipariþ zamaný geldiyse... === BURADAKÝ ÝF BLOÐU RASTGELE SEÇÝLECEK MÜÞTERÝ VE SÝPARÝÞ ÝÇÝN KONTROL EDER ===  MÜÞTERÝ EKRANDAN KAYBOLDUKTAN 4-9 SANÝYE SONRA YENÝ MÜÞTERÝ VE SÝPARÝÞ SEÇÝLÝR ===
                if (siparisSonrasiBekleme && gecenSure.getElapsedTime().asSeconds() >= rastgeleBeklemeSuresi) {
                    // Yeni müþteri ve sipariþ seç
                    secilenMusteri = musteriDosyalari[rand() % musteriDosyalari.size()];
                    secilenSiparis = siparisMetinleri[rand() % siparisMetinleri.size()];

                    siparisUrunleri.clear();
                    if (secilenSiparis.find("Kola") != std::string::npos)
                        siparisUrunleri.push_back("Kola");
                    if (secilenSiparis.find("Gazoz") != std::string::npos)
                        siparisUrunleri.push_back("Gazoz");
                    if (secilenSiparis.find("Tavuk Burger") != std::string::npos)
                        siparisUrunleri.push_back("Tavuk Burger");
                    if (secilenSiparis.find("Et Burger") != std::string::npos)
                        siparisUrunleri.push_back("Et Burger");
                    if (secilenSiparis.find("Margarita Pizza") != std::string::npos)
                        siparisUrunleri.push_back("Margarita Pizza");
                    if (secilenSiparis.find("Patates") != std::string::npos)
                        siparisUrunleri.push_back("Patates");
                    if (secilenSiparis.find("Sucuklu Pizza") != std::string::npos)
                        siparisUrunleri.push_back("Sucuklu Pizza");

                    if (!musteriTexture.loadFromFile("assets/images/halfcustomer/" + secilenMusteri)) {
                        std::cerr << "Yeni musteri yuklenemedi!\n";
                    }
                    musteri.setTexture(musteriTexture);
                    musteri.setColor(sf::Color::White);

                    siparisYazisi.setString(secilenSiparis);
                    balon.setColor(sf::Color::White);
                    poset.setColor(sf::Color::White);

                    alinanUrunler.clear();
                    verilenUrunler.clear();
                    siparisHazir = false;
                    siparisSonrasiBekleme = false;

                    siparisSesi.play();
                }

                if (bildirimSaati.getElapsedTime().asSeconds() >= 4.0f) {
                    bildirimYazisi.setString("");  // 4 saniye geçtiyse gizle === BÝLDÝRÝM YAZISINI EKRANDA GÖSTERMEK ÝÇÝN BÝR ZAMANLAYICI KULLANILIR ===
                }




                // Buton hover efekti === MOUSE POZÝSYONU ÝLE ONAYLA BUTONUN GLOBAL BOUNDS'INI KONTROL EDEREK HOVER EFEKTÝ UYGULANIR ===
                if (onaylaButonSprite.getGlobalBounds().contains(worldPos)) {
                    onaylaButonSprite.setColor(sf::Color(255, 255, 255, 180)); // Hafif þeffaflýk efekti
                }
                else {
                    onaylaButonSprite.setColor(sf::Color::White);
                }


                // Hover efekti === MOUSE POZÝSYONU ÝLE ÜRÜNLERÝN GLOBAL BOUNDS'INI KONTROL EDEREK HOVER EFEKTÝ UYGULANIR === FOR DÖNGÜSÜ KULLANILARAK BÜTÜN ÜRÜNLERE UYGULANIR ===
                for (auto& urun : urunler) {
                    if (urun->sprite.getGlobalBounds().contains(worldPos))
                        urun->sprite.setColor(sf::Color(255, 255, 255, 150));
                    else
                        urun->sprite.setColor(sf::Color::White);
                }

                // Sürükleme baþlangýcý
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    mouseBasili = true;
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    for (auto& urun : urunler) {
                        if (urun->sprite.getGlobalBounds().contains(mousePos)) {
                            suruklenenUrun = new sf::Sprite(urun->sprite);
                            fark = mousePos - urun->sprite.getPosition();
                            seciliUrun = urun;
                            break;
                        }
                    }
                }

                if (kasaBakiyesi > 1000.0f) {  // === KASA BAKÝYESÝ 1000.0 TL'yi geçtiyse DOSYA KONUMUNA KULLANICI ÝSMÝ VE BAKÝYESÝ ÝLE YAZDIRLIR ===
                    dosyayaYazdir(oyuncuIsmi, kasaBakiyesi);  // Dosyaya yazdýr
                }

                if (event.type == sf::Event::Closed) {
                    // Eðer oyun penceresi kapatýlýrsa, dosyaya yazdýrma iþlemi yapýlabilir
                    if (kasaBakiyesi > 1000.0f) {
                        dosyayaYazdir(oyuncuIsmi, kasaBakiyesi);  // Kazançlarý dosyaya yazdýr
                    }
                    window.close();
                }



                break;
            case OyunDurumu::AYARLAR:
                // Ayarlar iþlevleri === ÝÞLEVÝ YOK === 
                break;
            case OyunDurumu::NASIL_OYNANIR:   //=== NASIL OYNANIR DURUMU CASE Ý AÇILIR === === OYUNCU ÝLE ETKÝLEÞÝM ÝÇEREN BÝR ÝÞLEVÝ YOKTUR === === OYUNCUYA BÝLGÝ VEREN GÖRSEL EKRANA ÇÝZÝLÝR ===
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {  //=== KLAVYEDE ESCAPE TUÞUNA BASILDIÐINDA MENÜ EKRANINA DÖNÜLÜR ===
                    oyunDurumu = OyunDurumu::MENU;
                }
                break;
            }













        }







        // === CIZIM ===
        window.clear(); // === PENCEREYÝ TEMÝZLER ===

        // === OYUN DURUMUNA GÖRE GÖRSELLERÝ ÇÝZME ===  
        // === MENU OYUN DURUMUNDA MENÜ GÖRSELLERÝ ÇÝZÝLÝR ===
        if (oyunDurumu == OyunDurumu::MENU) {
            window.draw(menuSprite);
            window.draw(menuYaziSprite);
            window.draw(menuOynaSprite);
            window.draw(menuCikisSprite);
            window.draw(menuAyarlarSprite);
            window.draw(menuNasilOynanirSprite);

        }
        else if (oyunDurumu == OyunDurumu::OYUN) {  // ===OYUN DURUMUNDA OYUN GÖRSELLERÝ ÇÝZÝLÝR ===

            if (!isimAlindi) {      // === ÝSÝM ALINMAMIÞSA OYUN EKRANI AÇILMADAN ÖNCE KULLANICIDAN ÝSÝM BÝLGÝSÝ ALINIR DOSYA ÝÞLEMLERÝNDE KULLANILACAKTIR ===
                // Beyaz bir arka plan
                window.clear(sf::Color::White);

                // Siyah dikdörtgen (oyuncu ismi için)
                sf::RectangleShape dikdortgen(sf::Vector2f(400, 50));  // Dikdörtgen boyutlarý
                dikdortgen.setFillColor(sf::Color::Black);  // Siyah renk
                dikdortgen.setPosition(200, 250);  // Dikdörtgenin pozisyonu (orta)

                // Yazýyý çiz
                window.draw(dikdortgen);  // Dikdörtgeni çiz

                // Girilen ismi gösteren yazý
                sf::Text oyuncuIsimYazisi;
                oyuncuIsimYazisi.setFont(font);
                oyuncuIsimYazisi.setCharacterSize(24);
                oyuncuIsimYazisi.setFillColor(sf::Color::White);  // Yazý beyaz
                oyuncuIsimYazisi.setString("Oyuncu adinizi girin: " + oyuncuIsmi);  // Girilen ismi ekrana yazdýr
                oyuncuIsimYazisi.setPosition(220, 255);  // Yazýyý dikdörtgenin içine yerleþtir

                window.draw(oyuncuIsimYazisi);  // Yazýyý çiz
            }
            else {         // === ÝSÝM ALINDIYSA OYUN EKRANI AÇILIR VE OYUN GÖRSELLERÝ ÇÝZÝLÝR ===  
                // Oyun durumunda arka planý çiz
                window.draw(background);
                window.draw(musteri);
                for (auto& urun : urunler) {
                    window.draw(urun->sprite);

                }
                if (suruklenenUrun != nullptr) {
                    window.draw(*suruklenenUrun);
                }
                window.draw(kasa);
                window.draw(poset);
                window.draw(balon);
                window.draw(siparisYazisi);
                window.draw(onaylaButonSprite);
                window.draw(paraButonSprite);
                window.draw(bakiyeYazisi);
                if (!bildirimYazisi.getString().isEmpty())
                    window.draw(bildirimPaneli);

                window.draw(bildirimYazisi);

            }
        }
        else if (oyunDurumu == OyunDurumu::NASIL_OYNANIR) {  // === NASIL OYNANIR DURUMUNDA OYUNCUYA BÝLGÝ VEREN GÖRSEL ÇÝZÝLÝR ===
            window.draw(hakkindaSprite);
        }
        else if (oyunDurumu == OyunDurumu::AYARLAR) {

        }

        window.display(); // === PENCEREYÝ GÖSTERÝR ===
    }

    // === BELLEK TEMIZLIGI ===
    for (auto& urun : urunler)
        delete urun;
    if (suruklenenUrun != nullptr)
        delete suruklenenUrun;

    return 0;
}