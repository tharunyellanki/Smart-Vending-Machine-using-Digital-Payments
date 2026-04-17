#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

//To use only IMAP functions, you can exclude the SMTP from compilation, see ESP_Mail_FS.h.

#define WIFI_SSID "ThAruN's iQOO"

#define WIFI_PASSWORD "y24i6vt4"

//-----------setting IMAP paramet ers------

/* The imap host name e.g. imap.gmail.com for GMail or outlook.office365.com for Outlook */

#define IMAP_HOST "outlook.office365.com"

#define IMAP_PORT 993

#define AUTHOR_EMAIL "projectvending24@outlook.com"

#define AUTHOR_PASSWORD "AaBbCc$123"


/* Callback function to get the Email reading status */

void imapCallback(IMAP_Status status);    

void checkAmountReceived(std::vector<IMAP_MSG_Item> &msgItems, bool headerOnly);

/* The IMAP Session object used for Email reading */

IMAPSession imap;

Session_Config config;

/* Button Pins */
int item1 = 34;
int item2 = 12;
int motor1 = 13;
int motor2 = 14;
int motor3 = 35;
int motor4 = 32;
int next = 26;
int rst = 25;

/* Item values*/
int item1_value = 0;
int item2_value = 0;
int total_cost = 0;
bool received = false;

void setup()
{

  lcd.init();

  lcd.clear();

  lcd.backlight();

  lcd.setCursor(0, 0);

//  Serial.begin(9600);

  lcd.print("Connecting WiFi.");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)

  {

    delay(200);

  }

  lcd.clear();

  lcd.print("WiFi connected.");
  MailClient.networkReconnect(true);
  delay(2000);

  pinMode(item1, INPUT_PULLUP);
  pinMode(item2, INPUT_PULLUP);
  pinMode(rst, INPUT_PULLUP);
  pinMode(next, INPUT_PULLUP);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);

}

void loop()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("Choose items by ");
  lcd.setCursor(1, 1);
  lcd.print("using buttons.");

  while(!digitalRead(item1) && !digitalRead(item2)) {
//    Serial.println(digitalRead(item1));
    delay(200);
  }

  do{
    if(digitalRead(rst)){
      item1_value = 0;
      item2_value = 0;
      return;    
    }

    if(digitalRead(item1)){
      item1_value++;
      if(digitalRead(item1))
      {
        delay(100);
      }
    }

    if(digitalRead(item2)){
      item2_value++;
      if(digitalRead(item2))
      {
        delay(100);
      }
    }

    lcd.clear();
    lcd.print("A:");
    lcd.setCursor(2,0);
    lcd.print(item1_value);
    lcd.setCursor(0,1);
    lcd.print("B:");
    lcd.setCursor(2,1);
    lcd.print(item2_value);
    delay(100);
  }while(!digitalRead(next));

  total_cost = (item1_value + item2_value)*10;
  
  lcd.clear();
  lcd.print("Amount to pay");
  lcd.setCursor(0,1);
  lcd.print("Rs.");
  lcd.setCursor(3,1);
  lcd.print(total_cost);


  do{
     /** Enable the debug via Serial port

      none debug or 0
  
      basic debug or 1
  
      Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
  
    */
    imap.debug(1);

    /* Set the callback function to get the reading results */
    imap.callback(imapCallback);

    /** In case the SD card/adapter was used for the file storagge, the SPI pins can be configure from
     * MailClient.sdBegin function which may be different for ESP32 and ESP8266
     * For ESP32, assign all of SPI pins
     * MailClient.sdBegin(14,2,15,13)
     * Which SCK = 14, MISO = 2, MOSI = 15 and SS = 13
     * And for ESP8266, assign the CS pins of SPI port
     * MailClient.sdBegin(15)
     * Which pin 15 is the CS pin of SD card adapter
     */

    /* Declare the Session_Config for user defined session credentials */
    Session_Config config;

    /* Set the session config */
    config.server.host_name = IMAP_HOST;
    config.server.port = IMAP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;

    /* Define the IMAP_Data object used for user defined IMAP operating options. */
    IMAP_Data imap_data;

    /* Set the storage to save the downloaded files and attachments */
    imap_data.storage.saved_path = F("/email_data");

    /** The file storage type e.g.
     * esp_mail_file_storage_type_none,
     * esp_mail_file_storage_type_flash, and
     * esp_mail_file_storage_type_sd
     */
    imap_data.storage.type = esp_mail_file_storage_type_flash;

    /** Set to download headers, text and html messaeges,
     * attachments and inline images respectively.
     */
    imap_data.download.header = true;
    imap_data.download.text = true;
    imap_data.download.html = false;
    imap_data.download.attachment = false;
    imap_data.download.inlineImg = false;

    /** Set to enable the results i.e. html and text messaeges
     * which the content stored in the IMAPSession object is limited
     * by the option imap_data.limit.msg_size.
     * The whole message can be download through imap_data.download.text
     * or imap_data.download.html which not depends on these enable options.
     */
    imap_data.enable.html = false;
    imap_data.enable.text = true;

    /* Set to enable the sort the result by message UID in the decending order */
    imap_data.enable.recent_sort = true;

    /* Set to report the download progress via the default serial port */
    imap_data.enable.download_status = true;

    /* Header fields parsing is case insensitive by default to avoid uppercase header in some server e.g. iCloud
    , to allow case sensitive parse, uncomment below line*/
    // imap_data.enable.header_case_sensitive = true;

    /* Set the limit of number of messages in the search results */
    imap_data.limit.search = 1;

    /** Set the maximum size of message stored in
     * IMAPSession object in byte
     */
    imap_data.limit.msg_size = 512;

    /** Set the maximum attachments and inline images files size
     * that can be downloaded in byte.
     * The file which its size is largger than this limit may be saved
     * as truncated file.
     */
    imap_data.limit.attachment_size = 1024 * 1024 * 5;

    // If ID extension was supported by IMAP server, assign the client identification
    // name, version, vendor, os, os_version, support_url, address, command, arguments, environment
    // Server ID can be optained from imap.serverID() after calling imap.connect and imap.id.

    // imap_data.identification.name = "User";
    // imap_data.identification.version = "1.0";

    /* Set the TCP response read timeout in seconds */
    // imap.setTCPTimeout(10);

    /* Connect to the server */
    if (!imap.connect(&config, &imap_data))
    {
      return;
    }

    /** Or connect without log in and log in later

      if (!imap.connect(&config, &imap_data, false))
        return;

      if (!imap.loginWithPassword(AUTHOR_EMAIL, AUTHOR_PASSWORD))
        return;
    */

    // Client identification can be sent to server later with
    /**
     * IMAP_Identification iden;
     * iden.name = "user";
     * iden.version = "1.0";
     *
     * if (imap.id(&iden))
     * {
     *    Serial.println("\nSend Identification success");
     *    Serial.println(imap.serverID());
     * }
     * else
     *    MailClient.printf("nIdentification sending error, Error Code: %d, Reason: %s", imap.errorCode(), imap.errorReason().c_str());
     */

    /*  {Optional} */
    /* Open or select the mailbox folder to read or search the message */
    if (!imap.selectFolder(F("INBOX")))
        return;

    /*  {Optional} */
    /** Message UID to fetch or read e.g. 100.
     * In this case we will get the UID from the max message number (lastest message)
     */
    imap_data.fetch.uid = imap.getUID(imap.selectedFolder().msgCount());

    // if IMAP server supports CONDSTORE extension, the modification sequence can be assign to fetch command
    // Note that modsequence value supports in this library is 32-bit integer
    imap_data.fetch.modsequence = 123;

    // To fetch only header part
    // imap_data.fetch.headerOnly = true;

    // or fetch via the message sequence number
    // imap_data.fetch.number = imap.selectedFolder().msgCount();

    // if both imap_data.fetch.uid and imap_data.fetch.number were set,
    // then total 2 messages will be fetched i.e. one using uid and other using number.

    /* Set seen flag */

    // The message with "Seen" flagged means the message was already read or seen by user.
    // The default value of this option is set to false.
    // If you want to set the message flag as "Seen", set this option to true.
    // If this option is false, the message flag was unchanged.
    // To set or remove flag from message, see Set_Flags.ino example.

      imap_data.fetch.set_seen = true;

    /* Fetch or read only message header */
    // imap_data.fetch.headerOnly = true;

    /* Read or search the Email and close the session */

    // When message was fetched or read, the /Seen flag will not set or message remained in unseen or unread status,
    // as this is the purpose of library (not UI application), user can set the message status as read by set \Seen flag
    // to message, see the Set_Flags.ino example.
    MailClient.readMail(&imap);

    /* Clear all stored data in IMAPSession object */
    imap.empty();

    if(received){
      break;
    }
    
  }while(true);

  lcd.clear();
  lcd.print("Amount Received.");
  lcd.setCursor(0,1);
  lcd.print("Rs.");
  lcd.setCursor(3,1);
  lcd.print(total_cost);

  if(item1_value > 0){
    digitalWrite(motor1, HIGH);
    delay(item1_value*5000);
    digitalWrite(motor1, LOW);
  }

  if(item2_value > 0){
    digitalWrite(motor2, HIGH);
    delay(item2_value*5000);
    digitalWrite(motor2, LOW);
  }

  received = false;
  item1_value = 0;
  item2_value = 0;
  total_cost = 0;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Please Collect");
  lcd.setCursor(1,1);
  lcd.print("the Items");
  delay(2000);
}

/* Callback function to get the Email reading status */

void imapCallback(IMAP_Status status)

{

  /* Show the result when reading finished */

  if (status.success())

  {

    /* Print the result */

    /* Get the message list from the message list data */

    IMAP_MSG_List msgList = imap.data();

    checkAmountReceived(msgList.msgItems, imap.headerOnly());

    /* Clear all stored data in IMAPSession object */

    imap.empty();


  }

}


void checkAmountReceived(std::vector<IMAP_MSG_Item> &msgItems, bool headerOnly)


    { 

  for (size_t i = 0; i < msgItems.size(); i++) 

  {

    /* Iterate to get each message data through the message item data */

  IMAP_MSG_Item msg = msgItems[i];

String msg_flags = msg.flags;
//    Serial.println(msg_flags);
//    Serial.println(msg.subject);

   int found = msg_flags.indexOf("\Recent"); 
   // Serial.println(msg.from);rintln
   // Serial.println(found);
     if (found != -1) {
      String amount_str = "";
      for (int i=0; i < strlen(msg.subject); i++) {
        if (i < 4)           
          continue;

        if (msg.subject[i] == '.') {
          break;
        }

        amount_str += msg.subject[i]; 
      }

      int amount = amount_str.toInt();
//      Serial.println(amount);
      if (amount == total_cost)
        received = true;
    }
  }
}