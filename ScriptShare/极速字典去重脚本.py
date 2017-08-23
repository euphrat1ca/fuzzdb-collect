字典对于一个安全从业者的意义非常重要，尤其是渗透测试工程师，一份好的字典往往能带来意外的收获。
在当前各种裤子满天飞的环境下，快速整理有用信息并加入自己的字典已成为安全从业者的一个必备技能。
在处理大量单一字典变量的时候，我们往往会发现大量的重复，无论是什么原因造成的，对于一个强迫症来说，这是不能忍受的。
在一次hash爆破中，被爆hash类型是bcrypt，此时字典的去重工作就尤为重要了，字典不去重会严重影响爆破效率。
于是使用自己几年前写的一个基于数组遍历的字典去重工具对约1500万的一份字典进行了去重，可结果吓我一跳，整整一天一夜居然还没完成。
于是就有了下面的脚本。
在大字典中，会有各种各样的问题，比如这份字典可能是几份字典合成的，有中文、俄文、英文等等，甚至在实际操作中我发现了二进制、不可见字符串、句子等等。
这样即便去重，也仍然会有大量的无用条目在里面继续影响效率。
现实中遇到的字典一般不会超过1个G，而计算机内存却远远大于这个数字，所以一下把字典加载到内存进行处理的方式是完全可行的。
经字典完全载入内存后，利用正则匹配（这里很灵活，可以根据需求实时更改）“抓”出所有符合条件的条目，再进行去重。
数据量很大的情况下，用数组遍历的方式是无法完成工作的，这时就需要用到“哈希表”这个概念。
熟悉python我们知道，python的set的元素具有唯一性，利用该性质便可轻易达到去重的目的，最关键的是，set在cpython解释器的实现用了hashtable，所以效率奇高。
下面是一个demo，去重1500万的一个字典，消耗时间大约10s左右。

各位可以将自己手上的字典去重工具和这35行代码做个对比

#coding=utf-8
 
import sys, re, os
 
def getDictList(dict):
    regx = '''[\w\~\`\!\@\#\$\%\^\&\*\(\)\_\-\+\=\[\]\{\}\:\;\,\.\/\<\>\?]+'''
    with open(dict) as f:
        data = f.read()
        return re.findall(regx, data)
 
def rmdp(dictList):
    return list(set(dictList))
 
def fileSave(dictRmdp, out):
    with open(out, 'a') as f:
        for line in dictRmdp:
            f.write(line + '\n')
 
def main():
    try:
        dict = sys.argv[1].strip()
        out = sys.argv[2].strip()
    except Exception, e:
        print 'error:', e
        me = os.path.basename(__file__)
        print 'usage: %s <input> <output>' %me
        print 'example: %s dict.txt dict_rmdp.txt' %me
        exit()
 
    dictList = getDictList(dict)
    dictRmdp = rmdp(dictList)
    fileSave(dictRmdp, out)
     
if __name__ == '__main__':
    main()
