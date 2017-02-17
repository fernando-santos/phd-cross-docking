import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Polygon;
import java.awt.Stroke;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Random;
import java.util.StringTokenizer;

import javax.imageio.ImageIO;


public class Image {
	
	private int quality;


	private float edgeLenght;
	private float borderLenght;
	private int nodeLenght;
	private ArrayList<String> arquivos;
	private ArrayList<Node> nodes;
	private int numNodes;
	private int networkX;
	private int networkY;
	private int gap; // eu preciso do gap p desenhar um no na posicao 0,0 por exemlpo.
	
		
	public Image(int nNodes){
		// valores para a criação da image
		this.quality = 5; // tamanho final da rede de pixels. Se mudar aqui, tem que mudar o tamanho dos nos e borda... tentativa e erro.
		this.edgeLenght = 1.5F;
		this.borderLenght = 1.5F;
		this.gap = 5;
		this.nodeLenght = 3;
		this.numNodes = nNodes;
		
		arquivos = new ArrayList<String>();
	}
	
	
	private void getArquivos(String path){
		File file = new File(path);
		String[] aux;
		int i;
		aux = file.list();
		
		for (i = 0; i < aux.length; i++)
		{
			if (aux[i].endsWith(".txt"))
			{
				arquivos.add(new String(path+aux[i]));
				System.out.println("Adicionando arquivo: " + aux[i]);
			}
		}
	}
	
    private StringTokenizer nextLine(BufferedReader in) throws IOException{
    	String linha;
        do{
        	linha = in.readLine();
            if (linha == null) return null;
            else linha = linha.trim();
        } while ( linha.equals("") || (linha.charAt(0) == '#') );

        return new StringTokenizer(linha, " \t;\n");
    }

	
	private void leArquivo(String path){
        FileInputStream fin;
        BufferedReader in;
        StringTokenizer tokens;
        float x = 0, y = 0;
        int i;
        
        System.out.println("path = " + path);
        
		nodes = new ArrayList<Node>();
		ArrayList<Node> nodesAux = new ArrayList<Node>();
                
        try{
            fin = new FileInputStream(path);
            in = new BufferedReader(new InputStreamReader(fin));
            
            // removendo lixo.... bosta de arquivo de entrada! Precisa de tanto lixo? huahuahuah
            tokens = nextLine(in);
            tokens = nextLine(in);
            tokens = nextLine(in);
            tokens = nextLine(in);
            tokens = nextLine(in);
            tokens = nextLine(in);

			float maxX = -10000, minX = +10000, maxY = -10000, minY = +10000;
            
            // lendo as posicoes dos nos
            for (i = 0; i < this.numNodes; i++)
            {
            	tokens = nextLine(in);
            	tokens.nextToken(); // removendo o id
                x = Float.parseFloat(tokens.nextToken());
                y = Float.parseFloat(tokens.nextToken());
                System.out.println("i = " + i + "  x = " + x + "  y = " + y);
                nodesAux.add(new Node(x,y));

				if ( x < minX ) minX = x;
				if ( x > maxX ) maxX = x;
				if ( y < minY ) minY = y;
				if ( y > maxY ) maxY = y;
            }
			minX -= 5; minY -= 5;
			maxX += 5; maxY += 5;

			this.networkX = (int)( maxX - minX );
			this.networkY = (int)( maxY - minY );
            for (i = 0; i < this.numNodes; i++)
            {
				nodes.add(new Node(nodesAux.get(i).getX()-minX, nodesAux.get(i).getY()-minY));
			}
            
        }catch (IOException e) {
			// TODO: handle exception
        	System.out.println("Erro na leitura do arquivo... Finalizando.");
        	System.exit(0);
		}
	}
	
	
/*	public double getDist (double x1, double y1, double x2, double y2){
		return (Math.sqrt(Math.pow(x1, y1) + Math.pow(x2, y2)));
	}*/
	
		
	public void drawImageEPS(String nomeArquivo){
		int i;
		Stroke BS;
		
		nomeArquivo = nomeArquivo.replace(".txt", ".eps");
		System.out.printf(nomeArquivo);
		

		try {
			FileOutputStream outputStream = new FileOutputStream(nomeArquivo);
			
			// Create a new document with bounding box 0 <= x <= x + gap and 0 <= y <= x + gap.
			EpsGraphics2D g = new EpsGraphics2D("Example", outputStream, 0, 0, (int)(this.networkX*quality + this.gap*quality), (int)(this.networkY*quality + this.gap*quality));
			

			// colorindo o fundo
	        g.setColor(Color.white);
	        g.fillRect(0, 0, (int)(this.networkX*quality + this.gap*quality), (int)(this.networkY*quality+ this.gap*quality));

	        // adicionando borda
	        // a borda serve p vc ter uma nocao da rede... onde os caras estao
	        BS = new BasicStroke(borderLenght);
	        g.setColor(Color.black);
	        g.setStroke(BS);
	        g.drawRect(this.gap*quality/2, this.gap*quality/2, (int)(this.networkX*quality -1), (int)(this.networkY*quality-1));
	        
	        /* GRAFO COMPLETO NAO DAH P DESENHAR LINKS
	        // adicionando links entre os nos
	        BS = new BasicStroke(edgeLenght);
	        g.setColor(Color.gray);
	        g.setStroke(BS);
	        for (i = 0 ; i < this.numNodes ; i ++) {
	        	for (j = i + 1 ; j < this.numNodes ; j ++) {
	        		g.drawLine(
	        			(int)(nodes.get(i).getX()*quality + this.gap*quality/2),
	        			(int)((this.networkY*quality ) - (nodes.get(i).getY()*quality ) + this.gap*quality/2),
	        			(int)(nodes.get(j).getX()*quality + this.gap*quality/2),
	        			(int)((this.networkY*quality ) - (nodes.get(j).getY()*quality ) + this.gap*quality/2)
	        		);

	        	}
	        }
	        */
	  
	        
	        g.setColor(Color.black); // cores dos nos
	        // desenhar os nos
	        for (i = 0 ; i < this.numNodes ; i ++) {
	        	g.fillRect( 
	        		(int)((nodes.get(i).getX())*quality + this.gap*quality/2 -this.nodeLenght), 
	        		(int)((this.networkY*quality ) - (nodes.get(i).getY()*quality ) + this.gap*quality/2 -this.nodeLenght), 
	        		2*this.nodeLenght, 
	        		2*this.nodeLenght
	        	);
	        }
	        
	        // desenhando o primeiro com cor diferente
	        g.setColor(Color.red); // cor do no
        	g.fillRect( 
	        		(int)((nodes.get(0).getX())*quality + this.gap*quality/2 -this.nodeLenght), 
	        		(int)((this.networkY*quality ) - (nodes.get(0).getY()*quality ) + this.gap*quality/2 -this.nodeLenght), 
	        		2*this.nodeLenght, 
	        		2*this.nodeLenght
	        	);	        
        
	        g.setColor(Color.blue);	        
	        // desenhar o id dos nos
	        for (i = 0 ; i < this.numNodes ; i ++) {
	        			g.drawString(Integer.toString(i),
	        				(int)((nodes.get(i).getX())*quality + this.gap*quality/2 -this.nodeLenght),
	        				(int)((this.networkY*quality ) - (nodes.get(i).getY()*quality ) + this.gap*quality/2 -this.nodeLenght -2));
	        }
	
			// salvando o arquivo
			g.flush();
			g.close();
        
			}catch (IOException e) {
				// TODO: handle exception
				// exception de cu eh rola.
			}        
	}
	
/*	public float distancia(int id1, int id2){
		return (float)Math.sqrt(Math.pow( nodes.get(id1).getX() - nodes.get(id2).getX(), 2 ) +
				Math.pow( nodes.get(id1).getY() - nodes.get(id2).getY(), 2));
	}*/
	
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		if (args.length == 0){
			System.out.println("Parametro esperado:\n  1) quantidade de vertices do grafo");
			return;
		}
		
		
		Image image = new Image(Integer.parseInt(args[0]));
		
		// recupera todos os nomes dos arquivos que estao dentro da pasta
		image.getArquivos("../entrada/");
		int i;
		
		for (i = 0; i < image.arquivos.size(); i++){
			// para cada arquivo da pasta, fazer:
			image.leArquivo(image.arquivos.get(i)); // recupera informacoes do arquivo
			
			image.drawImageEPS(image.arquivos.get(i));
		}
		System.out.println("Finalizando o programa...");
	}
}
