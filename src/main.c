#include "app.h"
#include "config.h"

int main(int argc, char *argv[]) {
  AppConfig cfg = parse_args(argc, argv);
  AppState *app = app_init(&cfg);
  if (!app)
    return 1;

  app_run(app);
  app_cleanup(app);
  return 0;
}
