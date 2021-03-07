# MouseDB -- 基于hot-dump机制的分析型数据库


## TODO

该项目尚未完成。

## 需求

### 场景

分析型数据库多用于对用户的行为进行汇总，提供给运营部门进行分析，以对业务进行调整。也用于通过用户行为生成用户画像，以进行广告投放、内容推荐等。

##### 数据内容

一般导入该数据库的数据内容多为服务器日志，记录了前端对服务器的请求记录，相关的字段如：APPID、UA、IP、请求内容、API种类等，分别对应着用户的信息（如APP版本、手机型号）以及用户的行为（请求的内容）。

##### 导入

+ 相对于事务性数据库，分析型数据库对时效性要求不高。
+ 多为相隔一定时间段批量导入，导入量大。

##### 查询

+ 多使用sum操作，对应着业务中对某个行为的人数进行统计。
+ 多使用范围查找，对应着业务中对某个范围内的行为或人群进行挑选与分析。
+ 多使用join操作，对应业务中对不同时间的数据进行合并分析。

##### 特点

+ 受到业务场景的影响，比起事务型数据库，分析型数据库对数据的实时可靠性要求不高，也不需要进行频繁的删、改操作。
+ 导入的数据过了一段时间后，其需求量就大大减少，可以将其定义为冷数据。刚导入的数据，其需求量较大，可以将其定义为热数据。数据库的数据需要有冷热转换的过程。

### API

对用户暴露以下API，其他API封装在内部。

基本的数据库管理操作：

```
UserAction::create_a_database()  // 数据库创建
UserAction::show_database_names()  // 显示所有数据库
UserAction::drop_a_database()  // 删除数据库
UserAction::use_a_database()  // 打开数据库
```

基本的数据表管理操作：

```
UserAction::create_a_table()  // 创建数据表
UserAction::drop_a_table()  // 删除数据表
UserAction::insert()  // 添加数据
UserAction::query()   // 查询结果
```

## 设计

### 总体结构

<img src="https://pic2.zhimg.com/80/v2-09d6f22e0f10a6f7ab50c04d595456c6_720w.jpeg"/>

如图所示，DataFileHandler负责与磁盘交互，DatabaseMetaHandler和TableMetaHandler基于它对元数据进行管理，TableRowHandler基于它对表中的一行行数据进行管理，而HotData和ColdData基于TableRowHandler对冷热数据进行管理，TableHandler统一对表元数据（TableMetaHandler）、冷热数据（HotData，ColdData）进行管理，DatabaseHandler统一对元数据（DatabaseHandler）和表（TableHandler进行管理），最后把接口封装到了UserAction中对外暴露。

### 文件读写

关于文件读写的功能全部封装在**DataFIleHandler**类中，该类基于c++标准库<fstream>进行封装，提供了与文件交互的接口。

#### 初始化与管理

```
DataFileHandler::open()
DataFileHandler::close()
DataFileHandler:is_open()
```

通过上述三个接口进行关于文件的打开与关闭操作。在open函数中，使用者可以根据不同的读写场景，选择不同的打开模式。使用RAII的设计，在类析构时，检测文件打开状态对句柄进行close操作。

#### 文本文件

在该数据库中，一些文本文件，如存储元数据的JSON文件需要进行读取与序列化。因此提供了

```
DataFileHandler::read_all_text()
DataFileHandler::read_text_line()
DataFileHandler::append()
```

进行文本文件的读写。

#### 数据文件

数据文件是数据库的核心，数据库的所有数据都存储在数据文件中。数据以二进制的形式存储在数据文件中，因此所有的读写都需要以二进制的形式进行。对数据文件的所有读取都是随机读的，也就是通过计算每行的大小、读取第几行，可以精准定位到要读取的位置。

##### 分页

MySQL的Innodb存储引擎对数据做了分页操作，将数据文件分为多个页，每次读取都会读取一整个页进入缓存中。这样做有一定的原因，在磁盘中，数据一般不是“按需读取”，而是会进行预读，即使每次只读1字节的数据，磁盘也会从该位置往后读一定数量的数据。在许多操作系统中，磁盘一般会一次性读取4K的数据块。我们在设计数据库存储引擎的时候也需要遵循这个原理，进行预读。

我将数据文件分为多个页，从文件开头开始，每4096字节，即4KB为一个页。每次进行数据读取时，都会计算数据所属的页，将一整页读取进入缓存中，再读取想要的数据，这其中有很多细节需要注意。首先我需要设定页的缓存和淘汰机制，我查阅资料得知了LRU Cache的概念，并引入LRU Cache来对分页做管理。

在LRU缓存队列（本质上是链表）中，需要设定一个最大缓存页数。每次有一个数据块被get，就会检查缓存队列中是否有该数据块，如果有，则将数据块提到队列的最前面，并返回数据。如果没有，则从文件中读取后提到队列的最前，返回数据，并淘汰处于队列最后的内存块，若队列未满则无需淘汰。这样经常被使用的内存块就会一直呆在缓存中。而delete操作就是直接在链表中删除一个元素，在某一页数据进行了修改后，缓存的页就变成了脏页，需要在外部使用delete删除。这些实现都只要在内部进行封装，对外只要暴露一个get和delete的接口，外接并不用关心内部是怎么实现的，只管获取和删除就好。

再说回数据的读取，如果我要读取某一行数据，那么我首先需要获得那行数据的位置--包括起始位置与终止位置，再检查起始位置与终止位置是否处于同一个页中、是否跨越了多个页，并考虑各种边界情况，精准计算每一字节，这部分工作还是比较麻烦的。最后的结果就是实现了分页、缓存的操作。后来我得知，在分析型数据库而不是事务型数据库中，这样子做其实缓存的命中率并不高，于是我就把缓存页数设置为1，这样整个LRU缓存管理机制就**自适应**地变成一个每次读一页的预读机制。

##### 读取

最初只有一个读取接口：

```
DataFileHandler::read_lines_into_buffer()
```

负责读取行数据，设定起始读取地址、每行大小、读取行数，读取的过程都用到了上面说的预读机制，考虑各种分页的边界情况。返回一块内存的指针以及合法的行数：由于传入的读取行数可能超过文件的最大大小，所以对合法的行数进行计算。

后来在冷数据的处理过程中，有时候我只要读取一行的主键并序列化，而不需要读取剩下的信息，使用上述的接口虽然可以完成，但是会多读取一些没用的数据，于是使用另一个接口，可以更好地节省IO。

```
DataFileHandler::read_primary_key_into_buffer()
```



### 数据序列化

前面说到数据文件的读取，读取是把数据块读到内存中，这个数据块包含许多信息，因此需要将其进行反序列化。对DataFileHandler读取的数据进行反序列化的过程就由TableRowHandler完成，TableRowHandler执行的任务都是基于行的操作。

```
TableRowHandler::read_and_deserialize()
TableRowHandler::serialize_and_write()
```

两个接口，一个将传入的JSON对象序列化为关于行的二进制数据存入数据文件中；另一个将二进制数据读取到内存中，反序列化为JSON对象。

还有一些接口用于冷热数据在hot-dump过程中与磁盘的交互：

```
TableRowHandler::read_next_row_buffer_and_index()  // 读取内存块、索引数据
TableRowHandler::read_buffer_and_index_to_the_end()  // 功能同上，读到末尾
TableRowHandler::read_row_index() // 读取索引数据
TableRowHandler::read_row_buffer() // 读取内存块
TableRowHandler::write_rows() // 内存块写入磁盘
```

读取无非就两种：读取索引和读取内存块，这几个接口是为了hot-dump过程的不同场景而设计的。



### 元数据管理

使用DatabaseMetaHandler和TableMetaHandler对数据库和数据表的元数据管理，它们各自管理着一个JSON文件。设计的终极目的是：一切关于元数据的操作都封装在这两个类中，外部不需要直接对类内的JSON对象进行直接操作或者读取。

关于这两个部分的API基本都是各种各样的get和set的操作。get时返回const类型的引用，可以让对象只读，同时还可以减少复制的开销。

### 数据库管理与数据表管理

使用DatabaseHandler和TablesHandler分别实现对数据库和数据表的的管理。在我的设计中，当程序运行时，就会使用一个DatabaseHandler，它管理着所有的数据库，它对外暴露的接口很简单：

```
DatabaseHandler::create()
DatabaseHandler::open()
DatabaseHandler::drop()
DatabaseHandler::get_all_database_name()
```

这四个接口分别对应着数据库的创建、打开、删除、显示，和SQL语句中的create、use、drop、show相对应。

create和drop的过程都需要两部分工作，一部分是删除磁盘上的文件，另一部分是修改元数据。

对每个数据库进行open操作的时候，就会生成一个TablesHandler。TablesHandler管理着数据库内部的表，所有涉及表的操作都封装在它的内部，像插入、查询、hot-dump等操作，都需要TablesHandler的统一指挥。

TablesHandler对外暴露的接口不多，主要是数据表的基本操作，其他是一些get操作。

```
TablesHandler::create_table()
TablesHandler::drop_table()
TablesHandler::dump_table()  // hot-dump
TablesHandler::query()
```

TablesHandler内部的实现比较复杂，它管理着数据库内的所有表相关的操作，所以**表元数据、数据表的冷数据、热数据**（对应三个类）全部由它管理，它需要接受外接请求，然后对着三个类进行调度。像查询、删表、hot-dump的过程都是需要这三各类协调合作完成的，缺一不可，所以就统一由TablesHandler调度管理。TableHandlers对外暴露的接口很简单，内部实现却很复杂。

### 冷热数据的处理与hot-dump机制

#### 热数据

使用HotDataManager对热数据进行管理，支持针对热数据的插入、查询（目前只支持完全命中主键的查询）以及dump功能，后续考虑支持删除和更新。

HotDataManager会使用一个热数据文件来存储每一条到来的数据。每一条到来的数据会按顺序存入热数据文件中，通过HotDataManager内部的红黑树保持其有序。定义红黑树的一个节点为pair<主键，数据在文件中第几行>的形式，红黑树节点通过比较主键来比较大小，红黑树支持插入主键相同的数据。

```
HotDataManager::query()
HotDataManager::dump_to_cold_data()
HotDataManager::insert_rows()
```

热数据的查询，目前只支持完全命中主键的查询，查询过程就是在红黑树上查找的过程，高效。热数据的插入操作也很简单，首先把数据写入文件，再把数据存到红黑树上。这里的先后顺序有讲究，因为把数据先写到文件中，此时如果断电或者出现故障，后续恢复的时候是可以恢复这些数据的，但是如果先存到红黑树上，此时出现故障，后续就无法恢复这些数据了。

#### 热数据的数据恢复机制

在表元数据中会有一个字段，记录现在当前使用的热数据文件的文件名（该文件名以时间戳命名）。当数据库启动时，会通过该字段用TableRowHandler打开当前使用的热数据文件，从中逐行读取主键，恢复内存中的红黑树。遇到比如断电的情况就可以用这种方式恢复数据，这是非hot-dump时期断电的容灾机制。

#### 冷数据

使用ColdDataManager对冷数据进行管理，目前只支持完全命中主键的查询。

```
ColdDataManager::query()
```

前面说到表元数据中有一个字段存储当前使用的热数据文件名，同样的，表元数据中也会有一个字段存储当前使用的冷数据文件（以时间戳命名）。每次实例化一个ColdDataManager实例，它都会从冷数据文件中读取主键的数据，建立索引。由于冷数据是按主键从小到大的顺序存取的，因此可以使用一个顺序表存储其主键作为索引，通过二分查找的方法进行高效查询。出于内存大小方面的考虑，这里不会把所有的数据全部读到内存中做索引，而是分块索引，设定每隔n行存储一条数据。通过获取文件大小，除以每行大小得到冷数据文件的总数据行数，再进行计算就可以精确算出需要多长的顺序表来存储索引。

冷数据的查询目前只支持完全命中主键的查询。这里有一个细节需要注意，就是由于在我的设计中，数据库底层是支持插入主键相同的数据的，所以这里的分块二分查找需要做一些修改。对于顺序存储的数据，假设对块头第一行数据做索引，一般的二分查找，假如给了一个要求的范围，min<=x<=max，只要从主键值小于等于min的块开始扫描，在主键值大于等于max的块停止扫描。但是在这里，主键可能相等，则需要从主键值小于min的第一个块开始扫描，在主键值大于max的第一块停止扫描。

#### hot-dump机制

在表元数据中，有着这样的字段：

```json
"hot_data": {

    // 正在使用的热数据文件
    // 文件名是时间戳，当数据库正常退出时，值为null。
    // 若数据库开启时，该值不为null，说明上次数据库非正常退出
    // 那么需要重新读取数据，构建红黑树
    "current_used": "1231233.hot",

    // 在非hot-dump时期，该值为null
    // 在hot-dump时期，该值指向新的热数据文件
    // 该热数据文件会记录hot-dump时期到来的数据
    // 当hot-dump结束时，该值会成为上面的current_used的值
    // 然后new值恢复为null
    "new": "1231999.hot"
},
"cold_data": {
    "current_used": "1212331.mdb",

    // 在非hot-dump时期，该值为null
    // 在hot-dump时期，该值指向新的热数据文件
    // 该热数据文件会记录hot-dump时期到来的数据
    // 当hot-dump结束时，该值会成为上面的current_used的值
    // 然后new值恢复为null
    "new": "1212349.mdb",
    "history": [

        // 历史记录（包括正在使用的文件）
        // 文件名是时间戳
        // 记录历史文件可以实现数据库回滚的功能
        "1212331.mdb",
        "1212312.mdb",
        "1212321.mdb"
    ]
}
```

冷热数据部分都分别有两个字段，一个记录了正在使用的数据文件，另一个记录了正在hot-dump过程中生成的新的数据文件。此外冷数据部分中还有历史记录。这些文件的文件名都是以时间戳命名的，可以直观地知道文件的生成时间。

hot-dump过程由TablesHandler控制，需要HotDataManager、ColdDataManager和TableMetaHandler的一同参与，它们分别负责热数据部分、冷数据部分以及表元信息。

hot-dump过程在子线程中进行，并且子线程与主线程是detach的，整个过程由TablesHandler控制。

1. TablesHandler调用HotDataManager中的dump_to_cold_data()函数
2. dump_to_cold_data()函数生成新的HotDataManager，并将对应的新热数据文件添加到表元数据的\["hot_data"]["new"]字段中。
3. dump_to_cold_data()函数将this->is_dumping = true，这相当于一个标记。当这个标记为被置为true时，此后到来的热数据都会插入到新生成的HotDataManager中，而不会再插入到老的HotDataManager中。同样的，每次查询操作也都会检查这个标记，在大多数时候，此标记为False，那么进行查询时会从HotDataManager和ColdDataManager分别查询后做二路归并；但当hot-dump时这个标志被置为true时，此后的查询都会在新的HotDataManager、老的HotDataManager以及冷数据中查询，做三路归并。通过这样的设计，可以保证在hot-dump过程中仍然可以进行数据的插入与查询。
4. dump_to_cold_data()函数创建一个新的TableRowHandler用于管理新生成的冷数据文件。把这个新的冷数据文件的文件名，填写到表元数据的\["cold_data"]["new"]字段中。
5. dump_to_cold_data()函数对冷数据和热数据进行归并操作，其中冷数据和热数据的具体内容都存在磁盘中，所以每次冷热数据都只读一条数据到内存，进行比较后写入文件，就把这条数据的内存释放了，也就是说内存中永远只会有两条数据，一条热数据和一条冷数据。所以理论上无论数据量多大都可以正常地进行归并操作，而无需当心因为数据量过大内存装不下的情况。
6. dump_to_cold_data()函数完成归并后，把新的HotDataManager返回给TablesHandler。
7. TablesHandler调用老HotDataManager的一个函数，上一个锁，在该锁下无法进行查询和插入操作。
8. TablesHandler把TableMeta中，把\["cold_data"]["current_used"]和\["hot_data"]["current_used"]两个字段填写为新的热数据文件和新的冷数据文件。把前面填的\["hot_data"]["new"]和\["cold_data"]["new"]两个字段设置为null。
9. 对新的冷数据，生成新的ColdDataManager对新的冷数据进行管理。
10. TablesHandler中有两个map，记录着表名和对应表的ColdDataManager与HotDataManager，现在我们要把记录的ColdDataManager与HotDataManager更该为新的ColdDataManager与HotDataManager，并删除原来老的ColdDataManager与HotDataManager。
11. 经过更改后，7中上的锁就自动解锁了(尽管子线程和主线程是分离的)，因为此时检查的不是老HotDataManager的is_dumping 字段，而是新的HotDataManager的is_dumping 字段，该字段为False，所以说切换后自动解锁了。

在这样的设计下，hot-dump过程仍然支持数据插入与查询，只有最后在老Manager向新Manager切换的瞬间有一个及其短暂的锁。并且在这样的设计下可以很好地实现hot-dump的容灾以及基于时间的数据库回滚能力。

#### 数据库回滚

在hot-dump过程中，生成新的冷数据后，老的冷数据并不会被删除，而是加入到了表元数据的\["cold_data"]["history"]字段中，因此我们只要把表元数据中的“正在使用”的冷数据文件名，改为要回滚的冷数据文件名，就可以实现数据的回滚了。

#### hot-dump过程的容灾

如果在hot-dump过程中出现了断电或意外停止的情况，下一次数据库打开时，会检查到\["hot_data"]["new"]和\["cold_data"]["new"]这两个字段不为空，这就意味着在hot-dump过程出现了故障。首先程序会找到\["hot_data"]["current_used"]找到参与hot-dump的热数据文件，再通过\["cold_data"]["current_used"]找到参与hot-dump的冷数据。对这对热数据和冷数据再次进行归并操作，完成尚未完成的归并操作。然后再通过\["hot_data"]["new"]字段找到新的热数据文件，将其换为"current_used"中的字段，然后在内存中恢复有序的索引。这样hot-dump过程即使出现故障也可以恢复，就算在恢复过程中又出现了故障，下次也可以继续恢复。
