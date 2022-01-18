cd ..
javac -cp .\tools\* .\src\main\java\*.java -d .\bin\main\java
java -cp ".\tools\*;.\bin\main\java" main.java.NodeTest
