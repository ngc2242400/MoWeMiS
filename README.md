# MoWeMiS
Om de bouwhandleiding te volgen is geen voorkennis vereist, maar enige technische kennis is wel een groot pluspunt.
Met behulp van sensoren worden een aantal weer en milieu gerelateerde variabelen gemeten. Die worden aangevuld met zowel weer als milieu data van het internet. De verzamelde gegevens worden als een geheel op een website weergeven en in een database opgeslagen. Alle gebruikte software is opensource.
De sensoren worden gecontroleerd door een Wemos D1 die zijn data doorgeeft via MQTT voor centrale verwerking door een Raspberry Pi 4. De Raspberry Pi haalt met Python scripts gegevens van het internet. Met Node-Red worden de sensor en internet data dan gecombineerd tot één dashboard dat aan de gebruiker gepresenteerd wordt als een website op de Pi.
