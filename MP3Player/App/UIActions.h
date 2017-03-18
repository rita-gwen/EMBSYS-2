#include "lcdUtil.h"

void InitUIMessageQueue(void);
void PostUIQueueMessage(uint8_t cmd, void* p_data, uint8_t dataSize);
void UIDispatcherTask(void* p_data);