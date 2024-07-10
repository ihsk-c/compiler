 このプログラムはプログラミング言語MPPLで記述されたプログラムを入力とし、CASL IIのプログラムを出力するコンパイラである。プログラム言語はC言語である。

・各フォルダの概要
src : 3つのソースファイル(.c)がある
include : 1つのヘッダファイル(.h)がある
object_files : srcにある各ソースファイルに対応するオブジェクトファイル(.o)がある
exe_file : 実行ファイルと入力ファイルのサンプルがある

・実行方法
コマンドラインにて以下のコマンドを入力する
./compiler_ex [入力ファイル]
例
./compiler_ex sample11pp.mpl

 出力されたCASL IIプログラムは以下のようなwebで公開されているシミュレータで試すことができる
https://www.chiba-fjb.ac.jp/fjb_labo/casl/casl2.cgi
