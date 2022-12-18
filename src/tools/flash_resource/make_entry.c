#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/endian.h>

/*
 * XXX TODO: put in a shared header file when we're done!
 */

#if 0
struct flash_resource_entry_header {
	uint32_t magic; /* magic value for checking */
	uint32_t checksum; /* crc32 checksum of whole thing */
	uint32_t type; /* type */
	uint32_t length; /* length of whole entry, incl header */
	uint32_t alignment; /* alignment of payload */
	uint32_t namelength; /* length of name field */
	uint32_t payload_length; /* length of payload */
	uint32_t rsv0; /* reserved */
};
#endif

#define		ENTRY_MAGIC	0x05091979
#define		HEADER_SIZE	(sizeof(uint32_t) * 8)
#define		ALIGNMENT	32

/*
 * Return the next aligned value from the given address + alignment.
 */
uint32_t
align_uint32(uint32_t val, uint32_t align)
{
	uint32_t v;

	v = (val + (align - (val % align)));

	return (v);
}

/*
 * Calculate the total buffer size given the payload length, name
 * length and desired alignment.
 */
size_t
calculate_buffer_size(size_t payload_len, size_t name_len, size_t alignment)
{
	size_t s;

	/* Initial - header, name */
	s = HEADER_SIZE;
	s += name_len;

	/* next, need to round up to the nearest alignment multiple */
	s = align_uint32(s, alignment);

	/* next, payload length */
	s += payload_len;

	/* next, alignment */
	s = align_uint32(s, alignment);

	/* done */
	return (s);
}

/*
 * Calculate header size given alignment constraints.
 */
size_t
calculate_header_size(size_t name_len, size_t alignment)
{
	size_t s;

	/* Initial - header, name */
	s = HEADER_SIZE;
	s += name_len;

	/* next, need to round up to the nearest alignment multiple */
	s = align_uint32(s, alignment);

	return (s);
}

/*
 * Calculate payload size given alignment constraints.
 */
size_t
calculate_payload_size(size_t payload_len, size_t alignment)
{
	size_t s;

	s = 0;

	/* payload length */
	s += payload_len;

	/* next, alignment */
	s = align_uint32(s, alignment);

	return (s);
}

char *
populate_header_buf(uint32_t type, const char *label, uint32_t payload_length,
    uint32_t length, size_t *header_buf_len, uint32_t alignment)
{
	char *buf, *p;
	size_t hdr_len;
	uint32_t val;

	hdr_len = calculate_header_size(strlen(label), ALIGNMENT);

	buf = calloc(1, hdr_len);
	if (buf == NULL) {
		warn("%s: calloc (%jd bytes)", __func__, hdr_len);
		return (NULL);
	}
	p = buf;

	/* Assemble header */
	val = htole32(ENTRY_MAGIC); memcpy(p, &val, sizeof(uint32_t)); p += sizeof(uint32_t);
	val = htole32(0) /* cksum */; memcpy(p, &val, sizeof(uint32_t)); p += sizeof(uint32_t);
	val = htole32(type); memcpy(p, &val, sizeof(uint32_t)); p += sizeof(uint32_t);
	val = htole32(length); memcpy(p, &val, sizeof(uint32_t)); p += sizeof(uint32_t);
	val = htole32(alignment); memcpy(p, &val, sizeof(uint32_t)); p += sizeof(uint32_t);
	val = htole32(strlen(label)); memcpy(p, &val, sizeof(uint32_t)); p += sizeof(uint32_t);
	val = htole32(payload_length); memcpy(p, &val, sizeof(uint32_t)); p += sizeof(uint32_t);
	val = htole32(0) /* rsv0 */; memcpy(p, &val, sizeof(uint32_t)); p += sizeof(uint32_t);

	/* Add string, it isn't NUL terminated */
	memcpy(p, label, strlen(label));

	*header_buf_len = hdr_len;

	return (buf);
}

/*
 * read the given payload into memory, return the buffer + actual
 * payload buffer size.
 */
static char *
payload_read(const char *fn, size_t *payload_buf_size, size_t *payload_size,
    size_t alignment)
{
	struct stat sb;
	char *ptr;
	size_t pb_size;
	int ret;
	int fd;

	/* open file */
	fd = open(fn, O_RDONLY);
	if (fd < 0) {
		warn("%s: open", __func__);
		return (NULL);
	}

	/* get file size */
	if (fstat(fd, &sb) < 0) {
		warn("%s: fstat", __func__);
		close(fd);
		return (NULL);
	}

	/* calculate buffer size w/ padding */
	pb_size = calculate_payload_size(sb.st_size, alignment);

	/* allocate memory */
	ptr = calloc(1, pb_size);
	if (ptr == NULL) {
		warn("%s: calloc (%jd bytes)", __func__, pb_size);
		close(fd);
		return (NULL);
	}

	/* read it all in - has to be all at once for now */
	ret = read(fd, ptr, sb.st_size);
	if (ret != sb.st_size) {
		warn("%s: read (not a full buffer read)", __func__);
		close(fd);
		free(ptr);
		return (NULL);
	}

	/* done */
	*payload_buf_size = pb_size;
	*payload_size = sb.st_size;
	close(fd);
	return (ptr);
}

void
usage(const char *progname)
{
	printf("%s: [-s source] [-d destination] [-t typeid] [-l label]\n",
	     progname);
	printf("\n");
	printf("\t-s <source> - source payload file\n");
	printf("\t-d <destination> - output file\n");
	printf("\t-t <typeid> - type field (32 bit integer)\n");
	printf("\t-l <label> - string label for lookup/naming\n");
}

int
main(int argc, char **argv)
{
	const char *dest = NULL, *src = NULL, *label = NULL, *typestr = NULL;
	const char *argv0;
	char *payload, *hdr;
	size_t payload_size, payload_buf_size, hdr_buf_size, size;
	ssize_t ret;
	uint32_t type;
	int fd;
	int ch;

	argv0 = argv[0];

	/* parse command line args */
	while ((ch = getopt(argc, argv, "s:d:t:l:h")) != -1) {
		switch (ch) {
		case 's':
			src = optarg;
			break;
		case 'd':
			dest = optarg;
			break;
		case 't':
			typestr = optarg;
			break;
		case 'l':
			label = optarg;
			break;
		case 'h':
		default:
			usage(argv0);
			exit(127);
		}
	}
	argc -= optind;
	argv += optind;

	if (src == NULL) {
		printf("ERROR: missing -s (source payload)\n");
		exit(127);
	}
	if (dest == NULL) {
		printf("ERROR: missing -d (destination filename)\n");
		exit(127);
	}
	if (typestr == NULL) {
		printf("ERROR: missing -t (type integer)\n");
		exit(127);
	}
	if (label == NULL) {
		printf("ERROR: missing -l (label)\n");
		exit(127);
	}

	type = strtoul(typestr, NULL, 0);

	/* read payload into a buf, buffer size is aligned */
	payload = payload_read(src, &payload_buf_size, &payload_size, ALIGNMENT);
	if (payload == NULL) {
		printf("ERROR: didn't manage to read the payload!\n");
		exit(1);
	}

	/* calculate total size */
	size = calculate_buffer_size(payload_size, strlen(label), ALIGNMENT);

	/* populate header contents, buffer size is also aligned */
	hdr = populate_header_buf(type, label, payload_size, size,
	    &hdr_buf_size, ALIGNMENT);
	if (hdr == NULL) {
		printf("ERROR: didn't manage to populate the header!\n");
		exit(1);
	}

	printf("hdr len=%jd, payload len=%jd, total len %jd\n",
	    hdr_buf_size, payload_buf_size, size);

	/* Time to write out our buffer */
	fd = open(dest, O_RDWR | O_CREAT | O_TRUNC);
	if (fd < 0) {
		err(1, "open");
	}

	/* write out our assembled buffer */
	ret = write(fd, hdr, hdr_buf_size);
	if (ret != hdr_buf_size) {
		err(1, "%s: write (size mismatch)", __func__);
	}
	ret = write(fd, payload, payload_buf_size);
	if (ret != payload_buf_size) {
		err(1, "%s: write (size mismatch)", __func__);
	}

	/* done! */

	close(fd);
	free(payload);
	free(hdr);
	return (0);
}
