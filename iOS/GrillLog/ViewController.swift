//
//  ViewController.swift
//
//  Copyright 2011-present Parse Inc. All rights reserved.
//

import UIKit
import Parse

class ViewController: UIViewController {

	@IBOutlet weak var grillContainer: UIView!
	@IBOutlet weak var grillLabel: UILabel!
	
	@IBOutlet weak var foodContainer: UIView!
	@IBOutlet weak var foodLabel: UILabel!
	
	@IBOutlet weak var lastUpdateLabel: UILabel!
	
	let formatter = NSDateFormatter()
	var timer:NSTimer!
	
    override func viewDidLoad() {
        super.viewDidLoad()
		
		formatter.dateFormat = "M/d/yy h:mm:ss a"
		
		foodContainer.layer.cornerRadius = 10
		foodContainer.layer.borderColor = UIColor.whiteColor().CGColor
		foodContainer.layer.borderWidth = 5
		
		grillContainer.layer.cornerRadius = 10
		grillContainer.layer.borderColor = UIColor.whiteColor().CGColor
		grillContainer.layer.borderWidth = 5

		updateTemps()
		
		timer = NSTimer.scheduledTimerWithTimeInterval(10.0, target: self, selector: "updateTemps", userInfo: nil, repeats: true)
		
        // Do any additional setup after loading the view, typically from a nib.
    }
	
	func updateTemps(){
		let query = PFQuery(className: "TempData")
		query.orderByDescending("createdAt")
		query.getFirstObjectInBackgroundWithBlock {
			(object: PFObject?, error: NSError?) -> Void in
			if error != nil || object == nil {
				print("The getFirstObject request failed.")
			} else {
				let foodTemp = object!["foodTemp"] as? String
				let grillTemp = object!["grillTemp"] as? String
				let lastUpdate = object!.createdAt
				
				if foodTemp != nil && grillTemp != nil {
					self.foodLabel.text = "\(NSString(string: foodTemp!).intValue)"
					self.grillLabel.text = "\(NSString(string: grillTemp!).intValue)"
					self.lastUpdateLabel.text = self.formatter.stringFromDate(lastUpdate!)
				}
			}
		}
	}

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}

