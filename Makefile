TARGET = balls
TYPE = ps-exe

SRCS = \
$(wildcard source/*.cpp)

include third_party/nugget/psyqo/psyqo.mk
