FROM ubuntu:18.10

RUN apt-get update
RUN apt-get install -y g++ make libpcre3 libpcre3-dev libutfcpp-dev locales git

RUN sed -i -e 's/# ja_JP.UTF-8 UTF-8/ja_JP.UTF-8 UTF-8/' /etc/locale.gen 
RUN locale-gen
ENV LANG ja_JP.UTF-8  
ENV LANGUAGE ja_JP:ja
ENV LC_ALL ja_JP.UTF-8 

WORKDIR /root/

# RUN git clone https://github.com/dacodas/goo-processing /root/goo-processing
ADD goo-processing/src /root/goo-processing/src
ADD goo-processing/Makefile /root/goo-processing/Makefile
ADD goo-processing/results/trie /root/goo-processing/results/
RUN ["make", "-C", "/root/goo-processing/", "CFLAGS=-DGOO_TRIE_LOCATION_STRING=\"\\\"/root/goo-processing/results/trie\\\"\""]
RUN mkdir -p /var/lib/goo/data/dictionary-entries

EXPOSE 7081

CMD ["/root/goo-processing/bin/process-trie"]
