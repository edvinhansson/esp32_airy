# Proof of Concept för säker visualisering av sensordata

# Inledning
Denna Proof of Concept dokumenterar en lösning för att säkert transportera sensorvärden från en sensor till ett frontend gränssnitt. Implementationen använder sig av HTTPS med TLS för kommunikationen mellan dem olika enheterna. Sensorenheten uppdateras säkert via FOTA (Firmware Over The Air) och kan även fabrikåterställas eller återställas till tidiagre version ifall problem skulle uppstå. [1]

Inom denna Proof of Concept så används en ESP32 för att samla ihop sensordata, och en enhet som kör Docker Containers för Node-RED, InfluxDB, och Grafana för kommunikation, databas, och visualisering respektivt.

# Systemarkitektur
### Enhetsarkitektur
Den enhet som samlar ihop sensordata är uppbyggd av:
- ESP32 mickokontroller för hantering av sensor samt kommunikation.
- DHT11-sensor för insamling av temperatur- samt fuktighetsvärden.
- Batteri för kontinuerlig datasamling. [2]
- Firmware skriven i C med Espressifs esp-idf.

### Backend
Backend körs i en Docker-miljö och består av:
- Node-RED: För hantering av datan från sensorenhet och vidare kommunikation med andra delar i systemet.
- InfluxDB: En databas för att lagra sensorvärderna.
- Grafana: För visualisering av datan lagrad i databasen.

### Kommunikationsprotokoll
Hela systemet använder sig av HTTPS med TLS för att transportera datan mellan det olika delarna. TLS:en är självsignerad via en CA med privata nycklar.

### Säkerhet
- TLS via HTTPS: Säker kommunikation melland de olika komponenterna i systemet.
- FOTA: Möjlighet att uppdatera firmware:en på sensorenhet. Kan även återställas till tidigare version. [1]

### Drifttid
Då sensorenheten ska kunna använda batteri och låg strömförbrukning så behövs ett energisnålt firmware med möjlighet att använda power-saving mode under tiden den ej rapporterar data. [2]

## Cyber Resilience Act (CRA)
- Säkerhet-by-design: Systemet använder sig av TLS för säker kommunikation mellan dem olika komponenterna.
- Uppdaterbarhet: Uppdateringar kan ske via FOTA, med möjlighet att återställa firmware vid problem.
- Sårbarhetshantering: Via kontinuerlig support samt säkerhetsuppdateringar vid risker. Automatiserad analysering av källkod/firmware är även en möjlighet.

---

### Implementationsstrul
[1]: Detta var inkluderat i min Proof of Concept, men blev ej implementerat då jag ej kunde reducera storleken av min firmware till <1MB pga. Wi-Fi provisioning.<br>
[2]: Nån säkring i min dev-board gick när jag råkade plugga in batteriet åt fel håll, så kunde ej verifiera strömförbrukning eller mäta batterinivå.

