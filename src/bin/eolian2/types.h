#ifndef EOLIAN_GEN_TYPES_H
#define EOLIAN_GEN_TYPES_H

void eo_gen_types_header_gen(const char *eof, Eina_Strbuf *buf,
                             Eina_Bool full, Eina_Bool legacy);
Eina_Strbuf *eo_gen_class_typedef_gen(const char *eof);

#endif
