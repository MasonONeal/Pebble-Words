#include <pebble.h>

const char* meanings[] = {"Keen insight","Lacking originality","Learned or scholarly","Clearly understood","Inclined to silence","Secret or confidential"};
const char* words[] = {"Acumen","Banal","Erudite","Lucid","Taciturn","Esoteric"};

float timer_delay = 1;
int currentIndex = 0;
bool word = false;
int listSize = 5;
int score = 5;

static char message[256] = "";

Window *window;
TextLayer *textLayer;

static BitmapLayer *image_layer;
static GBitmap *image;
static BitmapLayer *image_layer2;
static GBitmap *image2;

static void updateMessageWithFinalScore()
{
  snprintf(message, sizeof(message), "Score: \n%i / %i", score, listSize);
  text_layer_set_text(textLayer, message);

  layer_remove_from_parent(bitmap_layer_get_layer(image_layer));
  layer_remove_from_parent(bitmap_layer_get_layer(image_layer2));
}

static void updateMessageWithIntroScreen()
{
  snprintf(message, sizeof(message), "Press Thumbs Up or shake to start Pebble Words");
  text_layer_set_text(textLayer, message);

  Layer* windowLayer = window_get_root_layer(window);
  layer_add_child(windowLayer, bitmap_layer_get_layer(image_layer2));
}

static void updateMessageWithCurrentMeaning()
{
  snprintf(message, sizeof(message), "%s", meanings[currentIndex-1]);
  text_layer_set_text(textLayer, message);

  Layer* windowLayer = window_get_root_layer(window);
  layer_add_child(windowLayer, bitmap_layer_get_layer(image_layer2));
}

static void updateMessageWithNextWord()
{
  snprintf(message, sizeof(message), "%s", words[currentIndex]);
  text_layer_set_text(textLayer, message);

  Layer* windowLayer = window_get_root_layer(window);
  layer_add_child(windowLayer, bitmap_layer_get_layer(image_layer));
}

void timer_call()
{
    if (timer_delay < 300*100 )
    {
        timer_delay=timer_delay/0.7;
        app_timer_register(timer_delay/100, timer_call, NULL);
    }
    else
    {
        timer_delay = 1;
    }
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed >= 0.01 && word == false && currentIndex !=0 && currentIndex <= listSize)
  {
    if (currentIndex<listSize)
    {
      word = true;
      updateMessageWithNextWord();
      currentIndex++;
      //timer_call();
    }
    else
    {
      word = false;
      updateMessageWithFinalScore();
    }
  }
}

void accel_int(AccelAxisType axis, int32_t direction)
{
   if (currentIndex<listSize)
    {
      word = true;
      updateMessageWithNextWord();
      currentIndex++;
      //timer_call();
    }
    else
    {
      word = false;
      updateMessageWithFinalScore();
    }
}

static void didPressButton(ClickRecognizerRef recognizer, void *context) {

  ButtonId button = click_recognizer_get_button_id(recognizer);

  if (button==BUTTON_ID_UP) {
    if (currentIndex<listSize)
    {
      word = true;
      updateMessageWithNextWord();
      currentIndex++;
      //timer_call();
    }
    else
    {
      word = false;
      updateMessageWithFinalScore();
    }
  }
  else if (button==BUTTON_ID_DOWN && word==true) {
    updateMessageWithCurrentMeaning();
    word = false;
    score--;
  }
}

static void clickConfigProvider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, didPressButton);
  window_single_click_subscribe(BUTTON_ID_DOWN, didPressButton);
  accel_tap_service_subscribe(accel_int);
//  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

static void windowLoad(Window *window) {
  textLayer = text_layer_create((GRect) { .origin = { 0, 30 }, .size = { 124, 88 } });

  // This needs to be deinited on app exit which is when the event loop ends
  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_UP_AND_DOWN);

  // The bitmap layer holds the image for display
  image_layer = bitmap_layer_create((GRect) { .origin = { 124, 3 }, .size = { 27, 146 } });
  bitmap_layer_set_bitmap(image_layer, image);
  bitmap_layer_set_alignment(image_layer, GAlignTopLeft);

  image2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_UP);
  image_layer2 = bitmap_layer_create((GRect) { .origin = { 124, 3 }, .size = { 27, 146 } });
  bitmap_layer_set_bitmap(image_layer2, image2);
  bitmap_layer_set_alignment(image_layer2, GAlignTopLeft);

  updateMessageWithIntroScreen();

  GFont myFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  text_layer_set_font(textLayer, myFont);
  text_layer_set_text_alignment(textLayer, GTextAlignmentCenter);

  Layer* windowLayer = window_get_root_layer(window);
  layer_add_child(windowLayer, text_layer_get_layer(textLayer));
}

static void windowUnload(Window *window) {
  text_layer_destroy(textLayer);
  gbitmap_destroy(image);
  bitmap_layer_destroy(image_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, clickConfigProvider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = windowLoad,
    .unload = windowUnload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
//  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();

  return 0;
}