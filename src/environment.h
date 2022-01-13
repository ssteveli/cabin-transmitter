#ifndef SRC_ENV_
#define SRC_ENV_

void environment_init();
void environment_loop();
int environment_read_temperature(float *f);
int environment_read_humidity(float *f);

#endif
