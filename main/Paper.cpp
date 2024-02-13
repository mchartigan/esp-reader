/*
 *  Paper.cpp: class function definition for Paper
 *  
 *  Author: Mark Hartigan
 *  Date Created: 2022 Jan 24
 */

#include "Paper.hpp"

using namespace std;

/*
 *  Declare constructor with file length
 */
Paper::Paper(uint8_t len) : fLen(len) {}

/*
 *  Actual initialization; breaks shit when this is performed before app_main()
 *  Initializes display and image buffer
 */
void Paper::post_boot_init() {
    // Read SD card on boot and get current page from HEAD (max_files=1)
    if (!SD.begin(SS, SPI, 4000000, "/sd", 1)) {  // start up SD card
        Debug("Failed to allocate SD card");
        return;
    };

    File head = SD.open("/HEAD");
    if (!head) {
        Debug("HEAD missing! Starting at beginning.\n");
        pgNum = 0;
        maxPg = 0;
    }
    else {
        // HEAD is of the format "XXXX XXXX\n", which corresponds to the current
        // page and the max page, in that order
        string name = "";
        while(head.available()) {
            char next = head.read();
            if (next == '\n') break;    // get first line (where current file is stored)
            name += next;
        }
        head.close();

        stringstream ss(name);
        string word1, word2;
        ss >> word1;                    // get current page
        ss >> word2;                    // get max page
        pgNum = stoi(word1);
        maxPg = stoi(word2);
    }

    DEV_Module_Init();                  // initializing DEV
    EPD_7IN5_V2_Init();                 // initializing and clearing screen
    DEV_Delay_ms(500);

    Paint_NewImage(page, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT, ROTATE_270, WHITE);

    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    imageSize = ((EPD_7IN5_V2_WIDTH % 8 == 0) ? (EPD_7IN5_V2_WIDTH / 8 ) : (EPD_7IN5_V2_WIDTH / 8 + 1)) * EPD_7IN5_V2_HEIGHT;
    if ((page = (UBYTE *)malloc(imageSize)) == NULL) {
        Debug("Failed to allocate image memory!\n");
    }
    else {
        current_page();                 // display currently listed page
        Debug("Initialization complete.\n");
        WAIT = false;
    }
}

/*
 *  Write text to the screen in pre-specified format
 */
void Paper::write(const char *str) {
    // call Paint_NewImage()
    Paint_SelectImage(page);
    Paint_Clear(WHITE);

    // draw the string
    Paint_DrawString_EN(0, 0, str, &Font16, WHITE, BLACK);

    // display the image
    EPD_7IN5_V2_Display(page);
    // DON'T put screen to sleep unless you're gonna call the INIT func after
    // EPD_7IN5_V2_Sleep();
}

/*
 *  Pass image on SD card to memory (avoids overrunning heap)
 */
bool Paper::write_image(string fname) {

    fname = "/" + fname;
    char c[fname.size() + 1];
    fname.copy(c, fname.size() + 1);
    c[fname.size()] = '\0';

    File file = SD.open(c);
    if (!file) {
#if USE_DEBUG
        cout << "Page " << pgNum << " missing!\n";
#endif
        return false;
    }

#if USE_DEBUG
    cout << "Page " << fname << endl;
#endif

    file.read(page, imageSize);
    file.close();                       // close file
    return true;
}

/*
 *  Pull file with name fname and display on e-Paper
 */
bool Paper::display_page(string fname) {

    /* // writing text from file
    string text;
    while(file.available()) {
            text += file.read();
    }
    file.close();

    char c2[text.size() + 1];
    text.copy(c2, text.size() + 1);
    c2[text.size()] = '\0';

    write(c2);
    */

    Paint_SelectImage(page);            // select image
    // try to write page
    if (!write_image(fname)) { return false; }
    EPD_7IN5_V2_Display(page);          // display page

    return true;
}

/*
 *  Rewrite HEAD to new page
 */
bool Paper::update_head(string file) {
    File head = SD.open("/HEAD", "w");
    if (!head) {
        cout << "HEAD missing!\n";
        return false;
    }

    stringstream ss;
    ss << file << " " << setfill('0') << setw(fLen) << maxPg << endl;
    string line = ss.str();

    char c[line.size() + 1];
    line.copy(c, line.size() + 1);
    c[line.size()] = '\0';

    head.print(c);
    head.close();
    return true;
}

/*
 *  Redisplay current page
 */
void Paper::current_page() {

    stringstream ss;
    string fname;
    ss << setfill('0') << setw(fLen) << pgNum;
    ss >> fname;

    display_page(fname);                // display the page
    update_head(fname);                 // update HEAD to reflect new page
}

/*
 *  Flip to the previous page in the book
 */
void Paper::prev_page() {

    WAIT = true;                        // block new commands
    if (pgNum > 0) pgNum--;             // decrement if not first page

    current_page();
    WAIT = false;                       // allow new commands
}

/*
 *  Flip to the next page in the book
 */
void Paper::next_page() {

    WAIT = true;                        // block new commands
    if (pgNum < maxPg) pgNum++;         // increment if not last page

    current_page();
    WAIT = false;                       // allow new commands
}