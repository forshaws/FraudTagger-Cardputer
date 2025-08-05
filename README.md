## ⌨️ M5Cardputer Fraud Detection with FraudTagger API ##

![Alt Text](fraudtagger_m5cardputer.png)

An application that uses M5Cardputer to interact with the FraudTagger ~1 Billion Token Foundation Model API for scoring email usernames for traces of machine generation and fraudulence. This project includes code for managing Wi-Fi connectivity, Entering usernames with the keyboard, making API requests, and displaying scores on the M5Cardputer's screen.

## 📧 Instructions 

You can install FraudTagger for Cardputer directly from M5Burner or by compiling the .ino file in Arduino Studio and buring directly to your M5Cardputer. The app requires a fraudtagger.cfg file on SD Card along with a number of background images. If images are not found the app may display a balck screen. M5Burner will install images and files on SD Card at burn time. Afterward eject the card and edit fraidtagger.cfg with your WiFi credentials and optional API key. 

## 🔐 Free API Usage & Fair Use

M5Cardputer-FraudTagger is free to use under the MIT license for personal, educational, and prototyping purposes.

The included Cardputer app connects to the hosted **FraudTagger API**, which is free to use for up to **10 email checks per day**.

Usage beyond this will require a valid API license key, please [view the API plans](https://toridion.com/fraudtagger) for license details for commercial use.

The source code may be modified and redistributed under the terms of the MIT license, but **abuse of the free API tier (e.g., illegal use, bulk use, bypassing limits or causing a nuisance to anyone)** may result in access being restricted without notice.

✅ TL;DR:
- You can use/modify/share the app
- API access is free up to 10/day
- For more, see [Pricing & Terms](https://toridion.com/fraudtagger)
