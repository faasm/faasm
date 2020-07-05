//
// Created by Joshua Heinemann on 05.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//
#ifndef FAASM_FAASM_SGX_WAMR_H
#define FAASM_FAASM_SGX_WAMR_H
#define GEN_NAME(NAME_1, NAME_2) #NAME_1 "_" #NAME_2

#define FAASM_FUNC(name, id) __attribute__((visibility("default"))) __attribute__((export_name(GEN_NAME(id,name)))) name()

#ifdef __cplusplus
extern "C" {
#endif

int faasmGetCurrentIdx();

#ifdef __cplusplus
};
#endif
#endif //FAASM_FAASM_SGX_WAMR_H
