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
import Data.Time.Clock

import Data.Maybe
-- import qualified Config as C


main = do
  args <- getArgs
  let (pathImg:pathCont:_) = args
  viewer pathImg pathCont

type Contour = [(Int, Int)]

data ContourTrea = ContourTrea {
                    contour :: Contour,
                    subTrea :: [ContourTrea]
                }
{-
readContourTree :: String -> IO ContourTrea
readContourTree s = do
    f <- readFile s
-}


readContour :: String -> IO Contour
readContour s = do
    f <- readFile s
    let (nStr:pointsStr) = lines f
        n = read nStr :: Int
        pointFromStr str = (round $ read s1, round $ read s2)
                    where (s1:s2:_) = words str
    return $ map pointFromStr pointsStr

drawContour pm gc c = 
    zipWithM (drawLine pm gc)  c (tail $ c++[head c])

viewer pathImg pathCont = do
  initGUI
  win <- windowNew
  win `onDestroy` mainQuit
  windowSetDefaultSize win 600 400
  windowSetPosition win WinPosCenter

  lastDrawTimeVar <- newIORef =<< getCurrentTime
  
  view <- imageViewNew
  scrollWin1 <- imageScrollWinNew view
  win `containerAdd` scrollWin1
  
  pixbuf <- pixbufNewFromFile pathImg
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
    time <- liftIO getCurrentTime
    lastDrawTime <-liftIO $ readIORef lastDrawTimeVar
    liftIO $ print $ diffUTCTime lastDrawTime time
    if diffUTCTime time lastDrawTime > 0.03 then do
        liftIO $ writeIORef lastDrawTimeVar time
        (x, y) <- eventCoordinates
        Just (Rectangle x0  y0 _ _) <- liftIO $ imageViewGetDrawRect view
        liftIO  (print (x, y) >> drawInfo ((round x) - x0) ((round y) - y0)) >> return False
    else return False

  
  let setImage img = do
        catchGError (do pixbuf <- pixbufNewFromFile img
                        w <- pixbufGetWidth pixbuf
                        h <- pixbufGetHeight pixbuf
                        pm <- pixmapNew (Nothing :: Maybe Pixmap) w h (Just 24)
                        gc <- gcNew pm
                        drawPixbuf pm gc pixbuf 0 0 0 0 (-1) (-1) RgbDitherNone 0 0
                        gcSetValues gc newGCValues{ foreground = Color 65535 0 0 }
                        drawLine pm gc (0, 0) (w, h)
                        cont <- readContour pathCont
                        drawContour pm gc cont
                        pixbuf2 <- liftM fromJust $ pixbufGetFromDrawable pm (Rectangle 0 0 (w-1) (h-1))
                        imageViewSetPixbuf view (Just pixbuf2) True
                        set win [ windowTitle := img ])
              (\_ -> print "error")


  setImage pathImg


  widgetShowAll win
  mainGUI
