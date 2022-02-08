#ifndef IMAGES_H
#define IMAGES_H


enum RET_CODES images_put_image_to(int fd_dst, const unsigned int slot, const char *const source);
enum RET_CODES images_export_image(int fd, const unsigned int slot, const char *const export_file_name);
enum RET_CODES images_simple_format(int fd, unsigned int slot);


#endif /* IMAGES_H */
