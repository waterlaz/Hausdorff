module Main where

import Graphics.UI.Gtk
import Graphics.UI.Gtk.ImageView
import Graphics.UI.Gtk.Gdk.Pixbuf
import System.Environment
import Control.Monad
import Control.Monad.State
import Control.Monad.IO.Class
import Data.IORef
import System.Glib.GError
import Graphics.UI.Gtk.Gdk.GC
import Data.Time.Clock
import System.FilePath.Posix
import Data.Array.ST
import Data.Array
import Control.Monad.ST.Safe

import Data.Maybe
-- import qualified Config as C


import System.IO.Unsafe

main = do
  args <- getArgs
  let (pathImg:pathCont:_) = args
  viewer pathImg pathCont

type Contour = [(Int, Int)]

data ContourTree = ContourTree {
                    contour :: Contour,
                    subTree :: [ContourTree]
                }

listTree (ContourTree c s) = [c] ++ concat (map listTree s)
                

readContourTree :: String -> IO ContourTree
readContourTree s = do
    f <- readFile (s </> "tree")
    let g (fName:nS:rest) = do
            c <- readContour (s </> fName)
            restL <- g rest
            return $ (c, read nS):restL
        g _ = return []
    contours <- g $ words f

    let recReadTree = do
            (c, n) <- gets head
            modify tail
            children <- mapM (\_ -> recReadTree) [1..n]
            return $ ContourTree {contour = c, subTree = children}
    return $ fst (runState recReadTree contours)
            

drawTree pm gc t = do
    drawContour pm gc (contour t)
    mapM_ (drawTree pm gc) (subTree t)


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



getContourMap :: ContourTree -> ((Int, Int), (Int, Int)) -> Array (Int, Int) Contour
getContourMap t b = runSTArray $ do
    m <- newArray b (contour t) 
    mapM_ (markInterior m) (listTree t)
    return m
  where
        markInterior :: STArray s (Int, Int) Contour -> Contour -> ST s ()
        markInterior m c = do
            let drawLine (x1, y1) (x2, y2) = drawLoop x1 y1
                        where
                          drawLoop x y = do
                            writeArray m (x, y) c
                            if (x /= x2 || y /=y2) then 
                                if ((x-x1)*(y2-y1)<(x2-x1)*(y2-y1) || y2==y1) then drawLoop (x+ signum (x2-x1)) y
                                                                  else drawLoop x (y+ signum (y2-y1))
                            else return ()
                    
            zipWithM_ drawLine c (tail c ++ [head c])
            


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

  tree <- readContourTree pathCont 
  let cMap = getContourMap tree ((0, 0), (w+2, h+2))


  let 
    drawInfo x y 
     | x>=0 && x<w && y>=0 && y<h  = do
        drawPixbuf pm gc pixbuf 0 0 0 0 (-1) (-1) RgbDitherNone 0 0
        gcSetValues gc newGCValues{ foreground = Color 65535 0 0 }
        --drawLine pm gc (x-10, y-1) (x+10, y+10)
        drawContour pm gc (cMap!(x, y))
        pixbuf2 <- liftM fromJust $ pixbufGetFromDrawable pm (Rectangle 0 0 (w-1) (h-1))
        imageViewSetPixbuf view (Just pixbuf2) True
     | otherwise = return ()


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
                        --cont <- readContour pathCont
                        tree <- readContourTree pathCont
                        --drawContour pm gc cont
                        drawTree pm gc tree
                        pixbuf2 <- liftM fromJust $ pixbufGetFromDrawable pm (Rectangle 0 0 (w-1) (h-1))
                        imageViewSetPixbuf view (Just pixbuf2) True
                        set win [ windowTitle := img ])
              (\_ -> print "error")


  setImage pathImg


  widgetShowAll win
  mainGUI
