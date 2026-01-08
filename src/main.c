#include "app.h"
#include "config.h"

int main(int argc, char *argv[]) {
  AppConfig config = parse_args(argc, argv);
  AppState *app = app_init(&config);
  if (!app)
    return 1;

  app_run(app);
  app_cleanup(app);
  return 0;
}
