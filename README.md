Build ffmpeg with OpenH264 on Mac

=================================

*This file helps to introduce one way to build ffmpeg with OpenH264*

=================================

1. To get ffmpeg for OS X, you first have to install •Homebrew.Execute the command below to install Homebrew


   `ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`


   `brew install automake fdk-aac git lame libass libtool libvorbis libvpx \`

   `opus sdl shtool texi2html theora wget x264 xvid yasm`
   
   
2. Add Environment variable to your $HOME/.bash_profile

   CFLAGS=``freetype-config --cflags``

   LDFLAGS=``freetype-config --libs``

   PKG_CONFIG_PATH=`$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig:/usr/X11/lib/pkgconfig`


3. Build your OpenH264 project


   3.1 obtain openh264 project to execute


   `git clone https://github.com/cisco/openh264.git`



   3.2 build OpenH264 to execute
   
   
   `cd openh264`


   `make`


   `sudo make install`


4. Build ffmpeg with OpenH264


   4.1 obtain ffmpeg project to execute


   `git clone http://source.ffmpeg.org/git/ffmpeg.git ffmpeg`


   4.2 Build ffmpeg to execute


   `cd ffmpeg`


   `./configure --prefix=/usr/local --enable-gpl --enable-libopenh264`


   `make`


   `make install`

