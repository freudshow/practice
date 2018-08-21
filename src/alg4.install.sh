#Create a directory /usr/local/algs4.
cd /usr/local
sudo mkdir algs4
sudo chmod 755 algs4

#Navigate to the subdirectory /usr/local/algs4.
cd algs4
pwd

#Download the textbook libraries from algs4.jar and the Java wrapper scripts from javac-algs4, javac-cos226, javac-coursera, java-algs4, java-cos226, and java-coursera.
sudo curl -O "https://algs4.cs.princeton.edu/code/algs4.jar"
sudo curl -O "https://algs4.cs.princeton.edu/linux/{javac,java}-{algs4,cos226,coursera}"
sudo chmod 755 {javac,java}-{algs4,cos226,coursera}
sudo mv {javac,java}-{algs4,cos226,coursera} /usr/local/bin

#Download DrJava from drjava.jar, the wrapper script from drjava, and the configuration file from .drjava.
sudo curl -O "https://algs4.cs.princeton.edu/linux/drjava.jar"
sudo curl -O "https://algs4.cs.princeton.edu/linux/drjava"
sudo chmod 755 drjava
sudo mv drjava /usr/local/bin
curl -O "https://algs4.cs.princeton.edu/linux/.drjava"
mv .drjava ~

#Download Findbugs 3.0.1 from findbugs.zip; our Findbugs configuration file from findbugs.xml; and the Findbugs wrapper scripts from findbugs-algs4, findbugs-cos226, and findbugs-coursera.
sudo curl -O "https://algs4.cs.princeton.edu/linux/findbugs.{zip,xml}"
sudo curl -O "https://algs4.cs.princeton.edu/linux/findbugs-{algs4,cos226,coursera}"
sudo unzip findbugs.zip
sudo chmod 755 findbugs-{algs4,cos226,coursera}
sudo mv findbugs-{algs4,cos226,coursera} /usr/local/bin

#Download PMD 5.8.1 from pmd.zip; our PMD configuration file from pmd.xml and the PMD wrapper scripts pmd-algs4, pmd-cos226, and pmd-coursera.
sudo curl -O "https://algs4.cs.princeton.edu/linux/pmd.{zip,xml}"
sudo curl -O "https://algs4.cs.princeton.edu/linux/pmd-{algs4,cos226,coursera}"
sudo unzip pmd.zip
sudo chmod 755 pmd-{algs4,cos226,coursera}
sudo mv pmd-{algs4,cos226,coursera} /usr/local/bin

#Download Checkstyle 8.2 from checkstyle.zip; our Checkstyle configuration files from checkstyle-algs4.xml, checkstyle-cos226.xml, and checkstyle-coursera.xml; and the Checkstyle wrapper scripts from checkstyle-algs4, checkstyle-cos226, and checkstyle-coursera.
sudo curl -O "https://algs4.cs.princeton.edu/linux/checkstyle.zip"
sudo curl -O "https://algs4.cs.princeton.edu/linux/checkstyle-suppressions.xml"
sudo curl -O "https://algs4.cs.princeton.edu/linux/checkstyle-{algs4,cos226,coursera}.xml"
sudo curl -O "https://algs4.cs.princeton.edu/linux/checkstyle-{algs4,cos226,coursera}"
sudo unzip checkstyle.zip
sudo chmod 755 checkstyle-{algs4,cos226,coursera}
sudo mv checkstyle-{algs4,cos226,coursera} /usr/local/bin









