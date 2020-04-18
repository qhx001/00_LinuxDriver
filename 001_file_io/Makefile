#立即变量bin表示最终要生成的可执行程序的名称
#立即变量objs表示生成可执行程序需要依赖的所有.o文件
#立即变量deps表示当前目录下已经存在的依赖文件（.d文件）
bin  := file_copy
objs := file_copy.o
deps := $(foreach f, $(objs), .$(f).d)
deps := $(wildcard $(deps))

all:$(objs)
	gcc -o $(bin) $^

#如果deps不为空，表示当前目录下存在依赖文件，我们需要包含这些依赖文件
ifneq ($(deps),)
  include $(deps)
endif

#编译生成.o文件，并为每个.c文件生成一个依赖文件.xxx.d
%.o:%.c
	gcc -Wp,-MD,.$@.d -o $@ -c $<

#通过执行make test命令来查看这些变量的值
test:
	@echo "bin=$(bin)"
	@echo "objs=$(objs)"
	@echo "deps=$(deps)"

#清除当前目录下所有.o文件和可执行程序
clean:
	rm $(objs) $(bin) -f

#清除当前目录下所有.o文件，可执行程序和所有的依赖文件
distclean:
	rm $(objs) $(bin) $(deps) -f