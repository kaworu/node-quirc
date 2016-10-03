#ifndef NODE_QUIRC_DECODE_H
#define NODE_QUIRC_DECODE_H
/*
 * node_quirc_decode.h - node-quirc decoding stuff
 */

#include <stddef.h> /* for size_t */
#include <stdint.h> /* for uint8_t */

struct nq_code_list;
struct nq_code;

struct nq_code_list	*nq_decode(const uint8_t *img, size_t imglen);
const char		*nq_code_list_err(const struct nq_code_list *list);
unsigned int		 nq_code_list_size(const struct nq_code_list *list);
const struct nq_code	*nq_code_at(const struct nq_code_list *list, unsigned int index);
void			 nq_code_list_free(struct nq_code_list *list);

const char	*nq_code_err(const struct nq_code *code);
int		 nq_code_version(const struct nq_code *code);
const char	*nq_code_ecc_level_str(const struct nq_code *code);
int		 nq_code_mask(const struct nq_code *code);
const char	*nq_code_data_type_str(const struct nq_code *code);
const uint8_t	*nq_code_payload(const struct nq_code *code);
size_t		 nq_code_payload_len(const struct nq_code *code);

#endif /* ndef NODE_QUIRC_DECODE_H */
