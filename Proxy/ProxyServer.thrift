/***********************************************************
 * ProxyServer Thrift Script
 * Made By XinyuWang
 * 2015/10/15
 ***********************************************************/

namespace java proxyserver
namespace cpp proxyserver
namespace csharp ProxyServer

//"required" and "optional" keywords are purely for documentation.

service TProxyService {

  /**
   * ProxyGet API
   * @return Value as Binary
   */
  binary ProxyGet(
    1: required binary Trapdoor
  );

  /**
   * ProxyPut API
   */
  void ProxyPut(
    1: required binary Trapdoor,
    2: required binary Val,
    3: required binary IndexTrapdoor,
    4: required binary IndexVal
  );


  /**
   * ProxyGetColumn
   * @return a binary list of Column value
   */
  list<binary> ProxyGetColumn(
    1: required binary IndexTrapdoor,
    2: required binary IndexMask,
    3: required i32 GetNum
  );


  /**
   * EqualSearch1
   * @return a binary list of Key, with linear comparation
   */
  list<binary> EqualSearch1(

    1: required binary IndexTrapdoor,
    2: required binary IndexMask

  );


 /**
   * EqualSearch2
   * @return a binary list of Key, with fast index
   */
  list<binary> EqualSearch2(

    1: required binary IndexTrapdoor,
    2: required binary IndexMask

  );

  /**
   * OrderSearch
   * @return a binary list of Key which meet the requirement of order
   */
  list<binary> OrderSearch(

    1: required binary IndexTrapdoor,
    2: required binary OrderLeft

  );
  
  /**
   * Proxy RunRedis command
   * @return a binary list of the result
   */
  list<binary> RunCommand (
    1: required list<binary> command
  );
  
  /**
   * EqualSearch1 PlainText
   * @return a binary list of Key, with linear comparation
   */
  list<binary> EqualSearch1PlainText(

    1: required binary IndexTrapdoor,
    2: required binary IndexMask

  );
  
  /**
   * EqualSearch2 PlainText
   * @return a binary list of Key, with fast index
   */
  list<binary> EqualSearch2PlainText(

    1: required binary IndexTrapdoor,
    2: required binary IndexMask

  );
  
  /**
   * OrderSearch PlainText
   * @return a binary list of Key which meet the requirement of order
   */
  list<binary> OrderSearchPlainText(

    1: required binary IndexTrapdoor,
    2: required binary OrderLeft

  );

}
