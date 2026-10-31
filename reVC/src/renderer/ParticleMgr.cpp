//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "ParticleMgr_backend.h"
static_assert(sizeof(work_buff)==PARTICLE_MGR_WORK_BUFFER_SIZE,"Actual shared work buffer extent");
uint8_t *ParticleMgr_BackendWorkBuffer(void)
{
    // Borrow the original shared main-module work buffer without copying or owning it
    return work_buff;
}
//- rouz edit (ChatGPT)
