module Main where

import Graphics.UI.Gtk
import Graphics.UI.Gtk.ImageView
import Graphics.UI.Gtk.Gdk.Pixbuf
import System.Environment
import Control.Monad
import Control.Monad.IO.Class
import Data.IORef
import System.Glib.GError
import Graphics.UI.Gtk.Gdk.GC

import Data.Maybe
-- import qualified Config as C


main = do
  args <- getArgs
  viewer $ head args

type Contour = [(Int, Int)]

data ContourTrea = ContourTrea {
                    contour :: Contour,
                    subTrea :: [ContourTrea]
                }

viewer path = do
  initGUI
  win <- windowNew
  win `onDestroy` mainQuit
  windowSetDefaultSize win 600 400
  windowSetPosition win WinPosCenter

  
  view <- imageViewNew
  scrollWin1 <- imageScrollWinNew view
  win `containerAdd` scrollWin1
  
  pixbuf <- pixbufNewFromFile path
  w <- pixbufGetWidth pixbuf
  h <- pixbufGetHeight pixbuf
  pm <- pixmapNew (Nothing :: Maybe Pixmap) w h (Just 24)
  gc <- gcNew pm

  let drawInfo x y = do
        drawPixbuf pm gc pixbuf 0 0 0 0 (-1) (-1) RgbDitherNone 0 0
        gcSetValues gc newGCValues{ foreground = Color 65535 0 0 }
        drawLine pm gc (x-10, y-1) (x+10, y+10)
        pixbuf2 <- liftM fromJust $ pixbufGetFromDrawable pm (Rectangle 0 0 (w-1) (h-1))
        imageViewSetPixbuf view (Just pixbuf2) True


  view `on` motionNotifyEvent $ do 
    (x, y) <- eventCoordinates
    Just (Rectangle x0  y0 _ _) <- liftIO $ imageViewGetDrawRect view
    liftIO  (print (x, y) >> drawInfo ((round x) - x0) ((round y) - y0)) >> return False

  
  let setImage img = do
        catchGError (do pixbuf <- pixbufNewFromFile img
                        w <- pixbufGetWidth pixbuf
                        h <- pixbufGetHeight pixbuf
                        pm <- pixmapNew (Nothing :: Maybe Pixmap) w h (Just 24)
                        gc <- gcNew pm
                        drawPixbuf pm gc pixbuf 0 0 0 0 (-1) (-1) RgbDitherNone 0 0
                        gcSetValues gc newGCValues{ foreground = Color 65535 0 0 }
                        drawLine pm gc (0, 0) (w, h)
                        pixbuf2 <- liftM fromJust $ pixbufGetFromDrawable pm (Rectangle 0 0 (w-1) (h-1))
                        imageViewSetPixbuf view (Just pixbuf2) True
                        set win [ windowTitle := img ])
              (\_ -> print "error")


  setImage path


  widgetShowAll win
  mainGUI
